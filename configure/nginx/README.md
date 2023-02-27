nginx basic
===========

# install&start

**install**

`sudo apt install nginx` on ubuntu

**start**

`sudo service nginx [start|restart|status|reload]` or

`sudo /etc/init.d/nginx start`


# configure

The config file located in `/etc/nginx/sites-available/`. Only one `default` file is here at beginning.

The steps of create a new config are as follow:

- create a new config file like `mysite.conf` under `/etc/nginx/sites-available`

- create a soft link under `/etc/nginx/sites/enabled` to the new config file

- restart nginx `service nginx restart`

The configure file's template is put as `default`, inlude multiple site access.

# TODO

- [ ] enable ssl
- [ ] enable php
- [ ] enable access log
