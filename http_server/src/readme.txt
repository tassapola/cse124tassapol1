
=Compilation=
make all

=Run=
usage: $ ./http_server <http port> <web root>
example: $ ./http_server 30000 /home/tassapol/webapps

=Command support=
1. HEAD
$ curl -XHEAD http://localhost:30000/page1.html -v

2. GET
a) normal
Use your browser to 
http://localhost:30000/page1.html

b) conditional GET with If-Modified-Since
$ curl -v -XGET -z "29 Oct 2010 09:00:15 GMT" http://localhost:30000/page1.html

3. TRACE
$ curl -XTRACE http://localhost:30000/page1.html

4. PUT
$ curl -v -XPUT -HContent-type:text/plain --data stuff:morestuff http://localhost:30000/putme.txt

5. DELETE
$ curl -v -XDELETE http://localhost:30000/putme.txt

=GET MIME support=
htm, html, txt, jpg, jpeg, gif

=Notes=
- PUT was tested with content-type of text/plain.

