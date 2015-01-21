Content-Type: multipart/mixed;
 boundary="boundary1"

This is a multi-part message in MIME format.
--boundary1
Content-Type: multipart/mixed;
	boundary="boundary2"

This is a multi-part message in MIME format.

--boundary2
Content-Type: application/msword; name="foo.doc"

foo

--boundary2
--boundary1
Content-Type: multipart/mixed;
	boundary="boundary3"

This is a multi-part message in MIME format.

--boundary3
Content-Disposition: attachment; filename="foo.doc"

foo

--boundary3
--boundary1
