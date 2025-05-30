FROM nginx:latest

# Install fcgiwrap and spawn-fcgi
RUN apt-get update && apt-get install -y fcgiwrap spawn-fcgi python3

# Copy your nginx config and CGI scripts into the image
COPY nginx.conf /etc/nginx/nginx.conf
COPY YoupiBanane /YoupiBanane

# Start fcgiwrap and nginx
CMD spawn-fcgi -s /var/run/fcgiwrap.socket -M 766 /usr/sbin/fcgiwrap && nginx -g 'daemon off;'