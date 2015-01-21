<?php
header('Transfer-Encoding: ascii, chunked');

print dechex(4)."; Some flags\r\n";
print "Test\r\n";
print dechex(0)."\r\n";
print "Some-Header: Test\r\n";
print "Some-Header2: Test2\r\n";
print "\r\n";
