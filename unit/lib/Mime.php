<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _model_Mime extends PHPUnit2_Framework_TestCase {
	function setUp() {
		require_once 'core/lib/Mime.class.inc';
	}
	
	function testParse() {
		$mime = Mime::parse(file_get_contents(dirname(__FILE__).'/_mime/mime-001-ocal.msg'));

		$this->assertEquals('1.0', $mime->getHeader('mime-version'));
		$this->assertEquals('7bit', $mime->getHeader('Content-Transfer-Encoding'));
		$this->assertEquals('7bit', $mime->getHeader('CONTENT-TRANSFER-ENCODING'));
		$this->assertEquals('7bit', $mime->getHeader('content-transfer-encoding'));
		$this->assertEquals('multipart/mixed', $mime->getHeader('content-type'));
		$this->assertEquals('9FSMI47LWHJ249XJPSWQQOAYWEFJ1AAP16N17BVQJ1B31ZSM6AY5MIZV1JMBIPZ', $mime->getHeader('content-type', 'boundary'));
		$this->assertEquals('flowed', $mime->getHeader('content-type', 'format'));
		
		$this->assertEquals(1, count($mime->getParts()));

		$expected = file_get_contents(dirname(__FILE__).'/_mime/mime-001-ocal.txt');
		$actual = $mime->getContent();
		$this->assertEquals($expected, $actual);
	}

	function testQuotedPrintableEncode() {
		$text = "foo \r\n bar =\0\t";
		$expected = "foo =0D=0A=\r\n bar =3D=00=09";
		
		$this->assertEquals($expected, Mime::quotedPrintableEncode($text));
		//$this->_diff($expected, Mime::quotedPrintableEncode($text));
		
		$expected = "abcdefghijklmnopqrstuvxyzabcdefghijklmnopqrstuvxyzabcdefghijklmnopqr=E4=\r\n=F6=E5=E4=F6=E5";
		$text = "abcdefghijklmnopqrstuvxyzabcdefghijklmnopqrstuvxyzabcdefghijklmnopqräöåäöå";
		$this->assertEquals($expected, Mime::quotedPrintableEncode($text));

		$expected = "abcdefghijklmnopqrstuvxyzabcdefghijklmnopqrstuvxyzabcdefghijklmnopq=E4=F6=\r\n=E5=E4=F6=E5";
		$text = "abcdefghijklmnopqrstuvxyzabcdefghijklmnopqrstuvxyzabcdefghijklmnopqäöåäöå";
		$this->assertEquals($expected, Mime::quotedPrintableEncode($text));

		$text = "abc \t";
		$expected = "abc =09";
		$this->assertEquals($expected, Mime::quotedPrintableEncode($text));

		$text = "abc  ";
		$expected = "abc =20";
		$this->assertEquals($expected, Mime::quotedPrintableEncode($text));
		
		$text = "DESCRIPTION;ENCODING-QUOTED-PRINTABLE:----- Original Message ----- \r\nAbc";
		$expected = "DESCRIPTION;ENCODING-QUOTED-PRINTABLE:----- Original Message ----- =0D=0A=\r\nAbc";
		$this->assertEquals($expected, Mime::quotedPrintableEncode($text));
	}

	function testEncodeHeader() {
		$text = "Foo =åäö bar";
		$expected = "=?utf-8?q?Foo =3D=C3=A5=C3=A4=C3=B6 bar?=";
		$actual = Mime::encodeHeader($text, 'iso-8859-1', 'utf-8');
		$this->assertEquals($expected, $actual);
	}
	
	function testParseHeader() {
		$mime = Mime::parse("X-Example: Foo =?ISO-8859-1?Q?=3D?= =?ISO-8859-1?Q?=E5?= =?ISO-8859-1?Q?=E4?= =?ISO-8859-1?Q?=F6?= bar");
		$expected = Mime::charset("Foo = å ä ö bar", 'iso-8859-1');
		$actual = $mime->getHeader('X-Example');
		$this->assertEquals($expected, $actual);

		$mime = Mime::parse("X-Example: Protokoll =?ISO-8859-1?Q?arbetsm=F6te_1=2C_tr=E5dl=F6sa_n=E4t?=");
		$expected = Mime::charset("Protokoll arbetsmöte 1, trådlösa nät", 'iso-8859-1');
		$actual = $mime->getHeader('X-Example');
		$this->assertEquals($expected, $actual);
		
		$mime = Mime::parse(file_get_contents(dirname(__FILE__).'/_mime/mime-011-linewrapping.msg'));
		$actual = $mime->getHeader('Subject');
		$expected = Mime::charset("[Fwd: Ordererkännande: 3878048] #19109", 'iso-8859-1');
		$this->assertEquals($expected, $actual);
	}
	
	function testCharset() {
		$mime = Mime::parse(file_get_contents(dirname(__FILE__).'/_mime/mime-002-utf8.msg'));
		$expected = Mime::charset("åäö", 'iso-8859-1');
		$actual = $mime->getContent();
		$this->assertEquals($expected, $actual);
	}

	function testAttachments() {
		$mime = Mime::parse(file_get_contents(dirname(__FILE__).'/_mime/mime-004-attachments.msg'));
		$this->assertEquals('test', trim($mime->getContent()));

		$expected = array(array(
			'name' => 'foo5.txt',
			'data' => 'foo'));
		$this->assertEquals($expected, $mime->getAttachments());

		$mime = Mime::parse(file_get_contents(dirname(__FILE__).'/_mime/mime-005-attachments.msg'));
		$this->assertEquals('test', trim($mime->getContent()));

		$expected = array(array('name' => 'foo6.txt','data' => 'foo'));
		$this->assertEquals($expected, $mime->getAttachments());

		$mime = Mime::parse(file_get_contents(dirname(__FILE__).'/_mime/mime-006-attachments.msg'));
		$this->assertEquals('test', trim($mime->getContent()));

		$expected = array(array('name' => 'attachment.txt','data' => 'foo'));
		$this->assertEquals($expected, $mime->getAttachments());

		$mime = Mime::parse(file_get_contents(dirname(__FILE__).'/_mime/mime-007-attachments.msg'));
		$this->assertEquals("test\r\nText body", $mime->getContent());
		
		$expected = array(
			array('name' => Mime::charset('M1_trådlösa nät GU_050208.doc','iso-8859-1'),			'data' => 'foo'),
			array('name' => Mime::charset('Trådlösanät_Projektspecifikation_PA6.doc','iso-8859-1'),	'data' => 'foo'),
			);
		$this->assertEquals($expected, $mime->getAttachments());

		// Test message with attachment from Eudora forwarded using Pine
		$mime = Mime::parse(file_get_contents(dirname(__FILE__).'/_mime/mime-008-attachments.msg'));
		$expected = array(array('name' => Mime::charset('Ärendehantering.doc','iso-8859-1'), 'data' => 'foo'));
		$actual = $mime->getAttachments();
		$this->assertEquals($expected, $actual);
		
		$parts = $mime->getParts();
		array_shift($parts);
		$attachment = array_shift($parts);
		
		$this->assertEquals(Mime::charset('Ärendehantering.doc','iso-8859-1'), $attachment->isAttachment());
		$this->assertEquals(Mime::charset('Ärendehantering.doc','iso-8859-1'), $attachment->getHeader('Content-Type', 'name'));
		$this->assertEquals(Mime::charset('Ärendehantering.doc','iso-8859-1'), $attachment->getHeader('Content-Disposition', 'filename'));
		
		// Test message with attachment from Thunderbird forwarded using Pine
		$mime = Mime::parse(file_get_contents(dirname(__FILE__).'/_mime/mime-012-attachments.msg'));
		$expected = array(
			array('name' => Mime::charset('SD arbetsflöde mot Thirdline inkl funktioner.pdf','iso-8859-1'),				'data' => 'foo'),
			array('name' => Mime::charset('Ärendeflöde inom Servicedesk.pdf','iso-8859-1'),								'data' => 'foo'),
			array('name' => Mime::charset('Flöde i ord.pdf','iso-8859-1'),												'data' => 'foo'),
			array('name' => Mime::charset('Överblick av ärende rutin för Servicedesk mot  Thirdline.pdf','iso-8859-1'),	'data' => 'foo'),
			);
		$actual = $mime->getAttachments();
		$this->assertEquals($expected, $actual);
		
		// Test message with attachement but lacking filename value
		$mime = Mime::parse(file_get_contents(dirname(__FILE__).'/_mime/mime-013-attach-no-filename.msg'));
		$expected = array(
			array('name' => '', 'data' => 'foo'),
			);
		$actual = $mime->getAttachments();
		$this->assertEquals($expected, $actual);
		
		$actual = SyndLib::invoke($mime->getParts(), 'isAttachment');
		$expected = array(false, '');
		$this->assertEquals($expected, $actual);
		
		// Test plaintext extraction
		$actual = $mime->getMessageText();
		$expected = new MimeMultipart();
		$expected->setHeader('Content-Type', array('multipart/mixed', 'boundary' => '===============0966435334=='));
		
		$inner = new MimeTextpart('Test');
		$inner->setHeader('Content-Type', array('text/plain', 'charset' => 'iso-8859-1'));
		$inner->setHeader('Content-Transfer-Encoding', 'quoted-printable');
		$expected->addPart($inner);
		
		$this->assertEquals($expected, $actual);
		
		// Test Thunderbird headerencoding
		$mime = Mime::parse(file_get_contents(dirname(__FILE__).'/_mime/mime-014-attachment.msg'));
		$expected = array(
			array('name' => 'DATABASHuldasförslag061024 utformning.xls', 'data' => 'foo'),
			);
		$actual = $mime->getAttachments();
		$this->assertEquals($expected, $actual);
	}
	
	function testMessageText() {
		$mime = Mime::parse(file_get_contents(dirname(__FILE__).'/_mime/mime-009-plaintext.msg'));
		$actual = $mime->getMessageText();
		
		$inner = new MimeMultipart();
		$inner->setHeader('Content-Type', array('multipart/mixed', 'boundary' => 'boundary2'));
		
		$expected = new MimeMultipart();
		$expected->setHeader('Content-Type', array('multipart/mixed', 'boundary' => 'boundary1'));
		$expected->addPart($inner);
		
		$this->assertEquals($expected, $actual);

		// Test without Content-Disposition
		$mime = Mime::parse(file_get_contents(dirname(__FILE__).'/_mime/mime-010-plaintext.msg'));
		$actual = $mime->getMessageText();
		
		$inner = new MimeMultipart();
		$inner->setHeader('Content-Type', array('multipart/mixed', 'boundary' => 'boundary2'));
		$inner2 = new MimeMultipart();
		$inner2->setHeader('Content-Type', array('multipart/mixed', 'boundary' => 'boundary3'));
		
		$expected = new MimeMultipart();
		$expected->setHeader('Content-Type', array('multipart/mixed', 'boundary' => 'boundary1'));
		$expected->addPart($inner);
		$expected->addPart($inner2);
		
		$this->assertEquals($expected, $actual);
	}
	
	function testUnixNewlines() {
		$mime = Mime::parse(file_get_contents(dirname(__FILE__).'/_mime/mime-015-unix.msg'));
		$expected = "Test";
		$actual = $mime->getContent();
		$this->assertEquals($expected, $actual);
	}
	
	function testGeneration() {
		$part = new MimeTextpart('Test', '');
		$part->setHeader('Content-Type', "text/plain; charset=utf-8");
		$part->setHeader('Content-Transfer-Encoding', '8bit');
		
		$mime = new MimeMultipart(array($part));
		$mime->setHeader('Content-Type', array('multipart/mixed', 'boundary' => 'boundary'));
		
		$expected = file_get_contents(dirname(__FILE__).'/_mime/mime-016-mimepart.msg');
		$actual = $mime->toString();

		$this->assertEquals($expected, $actual);
	}
}
