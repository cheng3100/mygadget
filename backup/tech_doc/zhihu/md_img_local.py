#! /usr/bin/env python
import os
import re
import argparse
import requests
from pathlib import Path
from urllib.parse import urlparse

def download_image(url, output_dir):
    """Download image to local directory, return local path"""
    try:
        response = requests.get(url, stream=True, timeout=10)
        if response.status_code == 200:
            filename = os.path.basename(urlparse(url).path) or f"image_{hash(url)}.jpg"
            local_path = os.path.join(output_dir, filename)
            with open(local_path, 'wb') as f:
                for chunk in response.iter_content(1024):
                    f.write(chunk)
            return local_path
    except Exception as e:
        print(f"[ERROR] Failed to download {url}: {e}")
    return None

def process_single_markdown(md_path, image_dir):
    """Process single Markdown file"""
    with open(md_path, 'r', encoding='utf-8') as f:
        content = f.read()

    pattern = r'!\[.*?\]\((.*?)\)'
    matches = re.findall(pattern, content)
    updated = False

    for url in matches:
        if url.startswith(('http://', 'https://')):
            local_path = download_image(url, image_dir)
            if local_path:
                relative_path = os.path.relpath(local_path, os.path.dirname(md_path))
                content = content.replace(url, relative_path)
                updated = True

    if updated:
        with open(md_path, 'w', encoding='utf-8') as f:
            f.write(content)
        print(f"[SUCCESS] Updated: {md_path}")
    else:
        print(f"[SKIPPED] No external images found in: {md_path}")

def process_directory(md_dir, image_dir):
    """Process all Markdown files in directory recursively"""
    md_files = list(Path(md_dir).rglob('*.md'))
    if not md_files:
        print(f"[WARNING] No Markdown files found in: {md_dir}")
        return

    for md_path in md_files:
        process_single_markdown(str(md_path), image_dir)

def get_default_image_dir(input_path):
    """Get default image directory (./pic) relative to input path"""
    base_dir = os.path.dirname(input_path) if os.path.isfile(input_path) else input_path
    default_dir = os.path.join(base_dir, "pic")
    Path(default_dir).mkdir(parents=True, exist_ok=True)
    return default_dir

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Download external images in Markdown and replace with local paths."
    )
    parser.add_argument(
        "-f",
        required=True,
        help="Path to a Markdown file or directory containing Markdown files"
    )
    parser.add_argument(
        "-p",
        help="Directory to save downloaded images (default: './pic' in the same directory as -f)"
    )
    args = parser.parse_args()

    # Set default image directory if not specified
    image_dir = args.p if args.p else get_default_image_dir(args.f)

    if os.path.isfile(args.f) and args.f.endswith('.md'):
        process_single_markdown(args.f, image_dir)
    elif os.path.isdir(args.f):
        process_directory(args.f, image_dir)
    else:
        print(f"[ERROR] Invalid input path: {args.f}. Must be a Markdown file or directory.")
        exit(1)
