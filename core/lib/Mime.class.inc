<?php

abstract class MimeHeader {
    /**
     * Encodes a header as per RFC2047
     *
     * @param string $name     The header name
     * @param string $value    The header data to encode
     * @param string $charset  Character set name
     * @param string $encoding Encoding name (base64 or quoted-printable)
     * @param string $eol      End-of-line sequence. Default: "\r\n"
     *
     * @return string          Encoded header data (without a name)
     * @access public
     * @since 1.6.1
     */
    static function encodeHeader($name, $value, $charset='ISO-8859-1', $encoding='quoted-printable', $eol="\r\n", $depth=0) {
         if (is_array($value)) {
                $result = null;
                foreach ($value as $key => $parameter) {
			if (null != $result)
                        	$result .= '; ';
                        if (is_string($key))
                                $result .= $key.'="';
                        $result .= self::encodeHeader($name, $parameter, $charset);
                        if (is_string($key))
                                $result .= '"';
                }
		return $result;
        }

        // Structured headers
        $comma_headers = array(
            'from', 'to', 'cc', 'bcc', 'sender', 'reply-to',
            'resent-from', 'resent-to', 'resent-cc', 'resent-bcc',
            'resent-sender', 'resent-reply-to',
            'return-receipt-to', 'disposition-notification-to',
        );
        $other_headers = array(
            'references', 'in-reply-to', 'message-id', 'resent-message-id',
        );

        $name = strtolower($name);

        if (in_array($name, $comma_headers)) {
            $separator = ',';
        } else if (in_array($name, $other_headers)) {
            $separator = ' ';
        }

        if (!$charset) {
            $charset = 'ISO-8859-1';
        }

        // Structured header (make sure addr-spec inside is not encoded)
        if (!empty($separator)) {
            $parts = self::_explodeQuotedString($separator, $value);
            $value = '';

            foreach ($parts as $part) {
                $part = preg_replace('/\r?\n[\s\t]*/', $eol . ' ', $part);
                $part = trim($part);

                if (!$part) {
                    continue;
                }
                if ($value) {
                    $value .= $separator==',' ? $separator.' ' : ' ';
                } else {
                    $value = $name . ': ';
                }

                // let's find phrase (name) and/or addr-spec
                if (preg_match('/^<\S+@\S+>$/', $part)) {
                    $value .= $part;
                } else if (preg_match('/^\S+@\S+$/', $part)) {
                    // address without brackets and without name
                    $value .= $part;
                } else if (preg_match('/<*\S+@\S+>*$/', $part, $matches)) {
                    // address with name (handle name)
                    $address = $matches[0];
                    $word = str_replace($address, '', $part);
                    $word = trim($word);
                    // check if phrase requires quoting
                    if ($word) {
                        // non-ASCII: require encoding
                        if (preg_match('#([\x80-\xFF]){1}#', $word)) {
                            if ($word[0] == '"' && $word[strlen($word)-1] == '"') {
                                // de-quote quoted-string, encoding changes
                                // string to atom
                                $search = array("\\\"", "\\\\");
                                $replace = array("\"", "\\");
                                $word = str_replace($search, $replace, $word);
                                $word = substr($word, 1, -1);
                            }
                            // find length of last line
                            if (($pos = strrpos($value, $eol)) !== false) {
                                $last_len = strlen($value) - $pos;
                            } else {
                                $last_len = strlen($value);
                            }
                            $word = self::encodeHeaderValue(
                                $word, $charset, $encoding, $last_len, $eol
                            );
                        } else if (($word[0] != '"' || $word[strlen($word)-1] != '"')
                            && preg_match('/[\(\)\<\>\\\.\[\]@,;:"]/', $word)
                        ) {
                            // ASCII: quote string if needed
                            $word = '"'.addcslashes($word, '\\"').'"';
                        }
                    }
                    $value .= $word.' '.$address;
                } else {
                    // addr-spec not found, don't encode (?)
                    $value .= $part;
                }

                // RFC2822 recommends 78 characters limit, use 76 from RFC2047
                $value = wordwrap($value, 76, $eol . ' ');
            }

            // remove header name prefix (there could be EOL too)
            $value = preg_replace(
                '/^'.$name.':('.preg_quote($eol, '/').')* /', '', $value
            );

        } else {
            // Unstructured header
            // non-ASCII: require encoding
            if (preg_match('#([\x80-\xFF]){1}#', $value)) {
                if ($value[0] == '"' && $value[strlen($value)-1] == '"') {
                    // de-quote quoted-string, encoding changes
                    // string to atom
                    $search = array("\\\"", "\\\\");
                    $replace = array("\"", "\\");
                    $value = str_replace($search, $replace, $value);
                    $value = substr($value, 1, -1);
                }
                $value = self::encodeHeaderValue(
                    $value, $charset, $encoding, strlen($name) + 2, $eol
                );
            } else if (strlen($name.': '.$value) > 78) {
                // ASCII: check if header line isn't too long and use folding
                $value = preg_replace('/\r?\n[\s\t]*/', $eol . ' ', $value);
                $tmp = wordwrap($name.': '.$value, 78, $eol . ' ');
                $value = preg_replace('/^'.$name.':\s*/', '', $tmp);
                // hard limit 998 (RFC2822)
                $value = wordwrap($value, 998, $eol . ' ', true);
            }
        }

        return $value;
    }

    /**
     * Explode quoted string
     *
     * @param string $delimiter Delimiter expression string for preg_match()
     * @param string $string    Input string
     *
     * @return array            String tokens array
     * @access private
     */
    static function _explodeQuotedString($delimiter, $string)
    {
        $result = array();
        $strlen = strlen($string);

        for ($q=$p=$i=0; $i < $strlen; $i++) {
            if ($string[$i] == "\""
                && (empty($string[$i-1]) || $string[$i-1] != "\\")
            ) {
                $q = $q ? false : true;
            } else if (!$q && preg_match("/$delimiter/", $string[$i])) {
                $result[] = substr($string, $p, $i - $p);
                $p = $i + 1;
            }
        }

        $result[] = substr($string, $p);
        return $result;
    }

    /**
     * Encodes a header value as per RFC2047
     *
     * @param string $value      The header data to encode
     * @param string $charset    Character set name
     * @param string $encoding   Encoding name (base64 or quoted-printable)
     * @param int    $prefix_len Prefix length. Default: 0
     * @param string $eol        End-of-line sequence. Default: "\r\n"
     *
     * @return string            Encoded header data
     * @access public
     * @since 1.6.1
     */
    static function encodeHeaderValue($value, $charset, $encoding, $prefix_len=0, $eol="\r\n")
    {
        // Generate the header using the specified params and dynamicly
        // determine the maximum length of such strings.
        // 75 is the value specified in the RFC.
        $encoding = $encoding == 'base64' ? 'B' : 'Q';
        $prefix = '=?' . $charset . '?' . $encoding .'?';
        $suffix = '?=';
        $maxLength = 75 - strlen($prefix . $suffix);
        $maxLength1stLine = $maxLength - $prefix_len;

        if ($encoding == 'B') {
            // Base64 encode the entire string
            $value = base64_encode($value);

            // We can cut base64 every 4 characters, so the real max
            // we can get must be rounded down.
            $maxLength = $maxLength - ($maxLength % 4);
            $maxLength1stLine = $maxLength1stLine - ($maxLength1stLine % 4);

            $cutpoint = $maxLength1stLine;
            $output = '';

            while ($value) {
                // Split translated string at every $maxLength
                $part = substr($value, 0, $cutpoint);
                $value = substr($value, $cutpoint);
                $cutpoint = $maxLength;
                // RFC 2047 specifies that any split header should
                // be seperated by a CRLF SPACE.
                if ($output) {
                    $output .= $eol . ' ';
                }
                $output .= $prefix . $part . $suffix;
            }
            $value = $output;
        } else {
            // quoted-printable encoding has been selected
            $value = self::encodeQP($value);

            // This regexp will break QP-encoded text at every $maxLength
            // but will not break any encoded letters.
            $reg1st = "|(.{0,$maxLength1stLine}[^\=][^\=])|";
            $reg2nd = "|(.{0,$maxLength}[^\=][^\=])|";

            if (strlen($value) > $maxLength1stLine) {
                // Begin with the regexp for the first line.
                $reg = $reg1st;
                $output = '';
                while ($value) {
                    // Split translated string at every $maxLength
                    // But make sure not to break any translated chars.
                    $found = preg_match($reg, $value, $matches);

                    // After this first line, we need to use a different
                    // regexp for the first line.
                    $reg = $reg2nd;

                    // Save the found part and encapsulate it in the
                    // prefix & suffix. Then remove the part from the
                    // $value_out variable.
                    if ($found) {
                        $part = $matches[0];
                        $len = strlen($matches[0]);
                        $value = substr($value, $len);
                    } else {
                        $part = $value;
                        $value = '';
                    }

                    // RFC 2047 specifies that any split header should
                    // be seperated by a CRLF SPACE
                    if ($output) {
                        $output .= $eol . ' ';
                    }
                    $output .= $prefix . $part . $suffix;
                }
                $value = $output;
            } else {
                $value = $prefix . $value . $suffix;
            }
        }

        return $value;
    }

    /**
     * Encodes the given string using quoted-printable
     *
     * @param string $str String to encode
     *
     * @return string     Encoded string
     * @access public
     * @since 1.6.0
     */
    static function encodeQP($str)
    {
        // Bug #17226 RFC 2047 restricts some characters
        // if the word is inside a phrase, permitted chars are only:
        // ASCII letters, decimal digits, "!", "*", "+", "-", "/", "=", and "_"

        // "=",  "_",  "?" must be encoded
        $regexp = '/([\x22-\x29\x2C\x2E\x3A-\x40\x5B-\x60\x7B-\x7E\x80-\xFF])/';
        $str = preg_replace_callback(
            $regexp, array('self', '_qpReplaceCallback'), $str
        );

        return str_replace(' ', '_', $str);
    }

    /**
     * Callback static function to replace extended characters (\x80-xFF) with their
     * ASCII values (RFC2047: quoted-printable)
     *
     * @param array $matches Preg_replace's matches array
     *
     * @return string        Encoded character string
     * @access private
     */
    static function _qpReplaceCallback($matches)
    {
        return sprintf('=%02X', ord($matches[1]));
    }
}

/**
 * Mime parser
 * @access		public
 * @package		synd.core.lib
 */
abstract class Mime {
	static function createTextpart($body = null, $filename = null, $charset = null) {
		return new MimeTextpart_2_0_0($body, $filename, $charset);
	}
	
	static function createMultipart($parts = array()) {
		return new MimeMultipart_2_0_0($parts);
	}

	/**
	 * Parses a MIME message
	 * @param	string		MIME encoded message
	 * @param	callback	Callback to determine if message should be saved verbatim
	 * @param	bool		Should the message be saved verbatim
	 * @return	MimePart
	 */
	static function parse($message, $callback = null, $verbatim = false) {
		$chunks = preg_split('/\r?\n\r?\n/', $message, 2);
		if (!isset($chunks[0]))
			return self::createTextpart();
		
		$headers = self::parseHeaders($chunks[0]);
		$lower = array_change_key_case($headers, CASE_LOWER);
		
		if (!isset($lower['content-type']) || false === ($i = strpos(SyndLib::reset(SyndLib::reset($lower['content-type'])),'/')))
			$mime = self::createTextpart();
		else {
			switch (strtolower(substr(SyndLib::reset(SyndLib::reset($lower['content-type'])), 0, $i))) {
                case 'multipart':
					$mime = self::createMultipart();
					break;
                
                // Encapsulated (forwarded) message
                case 'message':
					if (isset($chunks[1]))
						return self::parse($chunks[1]);
					$mime = self::createTextpart();
					break;

                default:
					$mime = self::createTextpart();
					break;
			}
		}
	
		$mime->_headers = array();
		foreach ($headers as $name => $values) {
			foreach ($values as $value)
				$mime->addHeader($name, $value, 'utf-8');
		}
		if (isset($chunks[1]))
			$mime->parse($chunks[1], $callback);
		if ($verbatim)
			$mime->setMessage($message);
		
		return $mime;
	}
	
	/**
	 * Parses and converts headers into UTF-8
	 * @param	string	The header message chunk
	 * @return	array
	 */
	static function parseHeaders($chunk) {
		preg_match_all('/^([\w\-]+): ?([^\r\n]*)/mS', 
			preg_replace('/\r?\n[\t ]+/S', ' ', 
			preg_replace('/=\r?\n[\t ]+/S', '=', $chunk)), $matches);
		$result = array();
		foreach ($matches[1] as $i => $name) {
			$value = self::parseHeader($matches[2][$i]);
			if (!isset($result[$name]))
				$result[$name] = array();
			$result[$name][] = $value;
		}
		return $result;
	}

	/**
	 * Returns values in UTF-8
	 */
	private static function parseHeader($value) {
		$result = self::_parseHeaderValue($value);
		/*
		if (preg_match_all('/;\s*([\w\-]+)(?:\s*[\*\d]*=\s*(")?(.*?)(?:(?<!\\\\)(?(2)"|(?=;))|$))?/S', $value, $matches)) {
			$result = array(substr($value, 0, strpos($value, ';')));
			foreach ($matches[1] as $i => $name) {
				if (isset($result[$name]))
					$result[$name] .= $matches[3][$i];
				else
					$result[$name] = $matches[3][$i];
			}
		}
		else
			$result = array($value);
		*/
		
		// Transport decode and convert to utf-8
		foreach ($result as $name => $value) {
			if (0 === stripos($value, "iso-8859-1''"))
				$value = self::charset(rawurldecode(substr($value, 12)), 'iso-8859-1', 'utf-8');
			$result[$name] = preg_replace_callback('/=\?([^?]+)\?(Q|B)\?([^?]*)\?=/iS', array('Mime','_callback_header_decode'), $value);
		}
		
		return $result;
	}

    /**
     * Function to parse a header value,
     * extract first part, and any secondary
     * parts (after ;) This function is not as
     * robust as it could be. Eg. header comments
     * in the wrong place will probably break it.
     *
     * @param string Header value to parse
     * @return array Contains parsed result
     * @access private
     */
    static function _parseHeaderValue($input)
    {

        if (($pos = strpos($input, ';')) === false) {
            return array(trim($input));
        }



        $value = substr($input, 0, $pos);
        $return = array(trim($value));
        $input = trim(substr($input, $pos+1));

        if (!strlen($input) > 0) {
            return $return;
        }
        // at this point input contains xxxx=".....";zzzz="...."
        // since we are dealing with quoted strings, we need to handle this properly..
        $i = 0;
        $l = strlen($input);
        $key = '';
        $val = false; // our string - including quotes..
        $q = false; // in quote..
        $lq = ''; // last quote..

        while ($i < $l) {
            
            $c = $input[$i];
            //var_dump(array('i'=>$i,'c'=>$c,'q'=>$q, 'lq'=>$lq, 'key'=>$key, 'val' =>$val));

            $escaped = false;
            if ($c == '\\') {
                $i++;
                if ($i == $l-1) { // end of string.
                    break;
                }
                $escaped = true;
                $c = $input[$i];
            }            


            // state - in key..
            if ($val === false) {
                if (!$escaped && $c == '=') {
                    $val = '';
                    $key = trim($key);
                    $i++;
                    continue;
                }
                if (!$escaped && $c == ';') {
                    if ($key) { // a key without a value..
                        $key= trim($key);
                        $return[] = trim($key);
                    }
                    $key = '';
                }
		else
	            $key .= $c;
                $i++;
                continue;
            }
                     
            // state - in value.. (as $val is set..)

            if ($q === false) {
                // not in quote yet.
                if ((!strlen($val) /*|| $lq !== false*/) && $c == ' ' ||  $c == "\t") {
                    $i++;
                    continue; // skip leading spaces after '=' or after '"'
                }
                if (!$escaped && ($c == '"' || $c == "'")) {
                    // start quoted area..
                    $q = $c;
                    // in theory should not happen raw text in value part..
                    // but we will handle it as a merged part of the string..
                    $val = !strlen(trim($val)) ? '' : trim($val);
                    $i++;
                    continue;
                }
                // got end....
                if (!$escaped && $c == ';') {

                    $val = trim($val);
                    $added = false;
                    if (preg_match('/\*[0-9]+$/', $key)) {
                        // this is the extended aaa*0=...;aaa*1=.... code
                        // it assumes the pieces arrive in order, and are valid...
                        $key = preg_replace('/\*[0-9]+$/', '', $key);
                        if (isset($return[$key])) {
                            $return[$key] .= $val;
                            $added = true;
                        }
                        // continue and use standard setters..
                    }
                    if (!$added) {
                        $return[$key] = $val;
                    }
                    $val = false;
                    $key = '';
                    $lq = false;
                    $i++;
                    continue;
                }

                $val .= $c;
                $i++;
                continue;
            }
            
            // state - in quote..
            if (!$escaped && $c == $q) {  // potential exit state..

                // end of quoted string..
                $lq = $q;
                $q = false;
                $i++;
                continue;
            }
                
            // normal char inside of quoted string..
            $val.= $c;
            $i++;
        }
        
        // do we have anything left..
        if (strlen(trim($key)) || $val !== false) {
           
            $val = trim($val);
            $added = false;
            if ($val !== false && preg_match('/\*[0-9]+$/', $key)) {
                // no dupes due to our crazy regexp.
                $key = preg_replace('/\*[0-9]+$/', '', $key);
                if (isset($return[$key])) {
                    $return[$key] .= $val;
                    $added = true;
                }
                // continue and use standard setters..
            }
            if (!$added) {
		if (strlen($val))
	                $return[$key] = $val;
		else
			$return[] = trim($key);
            }
        }
        return $return;
    }

	
	/**
	 * @access	private
	 */
	static function _callback_header_decode($matches) {
		switch (strtolower($matches[2])) {
			case 'b':
				return self::charset(base64_decode($matches[3]), $matches[1], 'utf-8');
			case 'q':
				return self::charset(quoted_printable_decode(str_replace('_',' ',$matches[3])), $matches[1], 'utf-8');
		}
	}
	
	private static function wordwrap($value) {
		if ($value != ($result = wordwrap($value, 74, "\r\n ")))
			return $result;
		return wordwrap($value, 74, "\r\n ", true);
	}

	/**
	 * Transer decodes a value
	 * @param	string	Value to decode
	 * @param	string	Encoding to use {base64, quoted-printable}
	 * @return	string
	 */
	static function decode($value, $encoding) {
		switch (strtolower($encoding)) {
			case 'base64':
				return base64_decode($value);
			case 'quoted-printable':
				return quoted_printable_decode($value);
		}
		return $value;
	}

	/**
	 * Transer encodes a value
	 * @param	string	Value to encode
	 * @param	string	Encoding to use {base64, quoted-printable}
	 * @return	string
	 */
	static function encode($value, $encoding) {
		switch (strtolower($encoding)) {
			case 'base64':
                return rtrim(chunk_split(base64_encode($value), 76, "\r\n"));
			case 'quoted-printable':
				return self::quotedPrintableEncode($value);
		}
		return $value;
	}

	/**
	 * Quoted printable encode
	 * @param	string	String to encode
	 * @param	integer	Width to wrap lines on
	 * @param	integer	Width to wrap the first line on
	 * @return	string
	 */
	static function quotedPrintableEncode($value, $line = 76, $first = 76) {
		$encoded = preg_replace_callback('/[^\x20-\x3C\x3E-\x7E]| $/S', array('Mime', '_encode_printable'), $value);
		if ($line != $first)
			$encoded = preg_replace('/^[^\r\n]{'.($first-5).','.($first-3).'}(?<!=)(?<!=[0-9A-F])(?!=[\r\n]|$)/S', "\\0=\r\n", $encoded);
		return preg_replace('/[^\r\n]{'.($line-5).','.($line-3).'}(?<!=)(?<!=[0-9A-F])(?!=[\r\n]|$)/S', "\\0=\r\n", $encoded);
	}
	
	/**
	 * @access	private
	 */
	static function _encode_printable($match) {
		if ("\n" == $match[0]{0})
			return "=0A=\r\n";
		return sprintf('=%02X', ord($match[0]{0}));
	}
	
	/**
	 * Converts between charsets (utf-8, iso-8859-1, ..)
	 * @param	string	String to convert
	 * @param	string	Source charset, defaults to ini_get('default_charset') or utf-8
	 * @param	string	Target charset, defaults to ini_get('default_charset') or utf-8
	 */
	static function charset($buffer, $from = null, $to = null) {
		if (null == $from && null == ($from = ini_get('default_charset')))
			$from = 'utf-8';
		if (null == $to && null == ($to = ini_get('default_charset')))
			$to = 'utf-8';
		if ($from != $to) {
			if (is_array($buffer)) {
				$result = array();
				foreach ($buffer as $key => $value) 
					$result[$key] = self::charset($value, $from, $to);
				return $result;
			}
			if ('utf-8' == strtolower($from) && 'iso-8859-1' == strtolower($to))
				return utf8_decode($buffer);
			if ('iso-8859-1' == strtolower($from) && 'utf-8' == strtolower($to) ||
				'windows-1252' == strtolower($from) && 'utf-8' == strtolower($to))
				return utf8_encode($buffer);
			return @mb_convert_encoding($buffer, $to, $from);
		}
		return $buffer;
	}
}

/**
 * Mime message part
 *
 * Headers are by default stored in UTF-8 internally and converted to
 * system charset on the fly, body is stored as received and 
 * converted to system charset on the fly.
 *
 * @access		public
 * @package		synd.core.model
 */
abstract class MimePart {
	/**
	 * @access	private
	 */
	var $_headers = array();

	/**
	 * Internal charset headers are stored in
	 */
	protected $_charset = 'iso-8859-1';
	
	/**
	 * Rendered message buffer
	 */
	protected $_message = null;
	
	function __construct($charset = 'utf-8') {
		$this->_charset = $charset;
	}
	
	function __clone() {}
	
	/**
	 * Adds a header
	 *
	 * The value might be a regular string value or an associative 
	 * array with additional header parameters
	 *
	 * <code>
	 * $mime = Mime::createTextpart('Test');
	 * $mime->setHeader('Content-Type', array('text/plain', 'charset' => 'utf-8', 'format' => 'flowed'));
	 * $mime->setHeader('Content-Transfer-Encoding', '8bit');
	 * </code>
	 *
	 * @param	string	Name of header
	 * @param	mixed	Value of header
	 * @param	string	Source charset, defaults to ini_get('default_charset') or utf-8
	 */
	function addHeader($name, $value, $charset = null) {
		if (!isset($this->_headers[$name]))
			$this->_headers[$name] = array();
		$this->_headers[$name][] = Mime::charset(is_array($value) ? $value : array($value), $charset, $this->_charset);
		$this->_message = null;
	}

	function setHeader($name, $value, $charset = null) {
		unset($this->_headers[$name]);
		$this->addHeader($name, $value, $charset);
	}

	/**
	 * Returns the headers (lowercase header names)
	 * @return	array
	 */
	function getHeaders() {
		return array_change_key_case(Mime::charset((array)$this->_headers, $this->_charset), CASE_LOWER);
	}

	/**
	 * Returns the value of a header
	 * @param	string	Name of header (eg. Content-Disposition)
	 * @param	string	Name of attribute (eg. filename)
	 * @return	string	Returns null if not found
	 */
	function getHeader($header, $attribute = null) {
		$name = strtolower($header);
		$headers = array_change_key_case((array)$this->_headers, CASE_LOWER);

		if (isset($headers[$name])) {
			if (null == $attribute)
				return Mime::charset(SyndLib::reset(SyndLib::reset($headers[$name])), $this->_charset);
			$attributes = array_change_key_case((array)SyndLib::reset($headers[$name]), CASE_LOWER);
			if (isset($attributes[$attribute]))
				return Mime::charset($attributes[$attribute], $this->_charset);
		}
		
		return null;
	}
	
	/**
	 * Returns the message parts
	 * @return	array
	 */
	function getParts() {
		return array();
	}

	/**
	 * Renders the MIME message into a string
	 * @return	string
	 */
	function toString() {
		if (null != $this->_message)
			return $this->_message;
		$message = $this->getRenderedHeaders();
		$message .= "\r\n";
		$message .= $this->getMessageBody();
		return $message;
	}
	
	/**
	 * Specify a verbatim message copy
	 * @param	string	The rendered message
	 */
	function setMessage($message) {
		$this->_message = $message;
	}
	
	/**
	 * Returns the outgoing message headers
	 * @return	array
	 */
	function getMessageHeaders() {
		return (array)$this->_headers;
	}
	
	/**
	 * Returns the rendered (outgoing) message headers
	 * @return	string
	 */
	function getRenderedHeaders() {
		$message = '';
		foreach ($this->getMessageHeaders() as $name => $values) {
			foreach ($values as $value)
				$message .= $name.": ".MimeHeader::encodeHeader($name, $value, $this->_charset)."\r\n";
		}
		return $message;
	}
	
	/**
	 * Renders the MIME message into a string without the headers
	 * @return	string
	 */
	abstract function getMessageBody();
	
	/**
	 * Returns the body payload
	 * @return	Should the content be implicitly charset normalized
	 * @return	string
	 */
	abstract function getContent($normalize = true);
	
	/**
	 * Extracts plain textparts (non attachments) from this message
	 * @param	Callback to determine if a part should be returned
	 * @return	MimePart
	 */
	function getMessageText($callback = null) {
		return $this;
	}

	/**
	 * Checks if this part is a file attachment
	 * @return	mixed	Returns false if not an attachment, the filename otherwise
	 */
	function isAttachment() {
		return false;
	}

	/**
	 * Returns file attachments
	 * <code>
	 * Format is:
	 *  array(
	 *	 array('name' => 'example.txt', 'data' => 'Example text'),
	 *	 array('name' => 'example2.txt', 'data' => 'Example2 text'),
	 *	)
	 * </code>
	 * @return	array
	 */
	function getAttachments() {
		return array();
	}

	/**
	 * Parses the body
	 * @access	protected
	 * @param	string		The body message chunk
	 * @param	callback	Callback to determine if message should be saved verbatim
	 */
	abstract function parse($chunk, $callback = null);
	
	/**
	 * Allows this message to be indexed effectivly
	 */
	function acceptIndexer($indexer) {
		$indexer->visitFragment($this->getHeader('From'));
		$indexer->visitFragment($this->getHeader('To'));
		$indexer->visitFragment($this->getHeader('CC'));
		$indexer->visitFragment($this->getHeader('Subject'));
	}
}

/**
 * Mime part such as text, html or file attachment
 */
class MimeTextpart_2_0_0 extends MimePart {
	var $_body = null;
	
	function __construct($body = null, $filename = null, $charset = null) {
		parent::__construct();
		if (null != $body) {
			if (null === $filename) {
				if (null == $charset && null == ($charset = ini_get('default_charset')))
					$charset = 'utf-8';
				$this->setHeader('Content-Type', array('text/plain', 'charset' => $charset));
				$this->setHeader('Content-Transfer-Encoding', '8bit');
				$this->_body = Mime::charset($body, null, $charset);
			}
			else {
				$this->setHeader('Content-Type', 'application/octet-stream');
				$this->setHeader('Content-Disposition', array('attachment', 'filename' => $filename));
				$this->setHeader('Content-Transfer-Encoding', 'base64');
				$this->_body = $body;
			}
		}
	}

	function getMessageBody() {
		return Mime::encode($this->_body, $this->getHeader('Content-Transfer-Encoding'));
	}

	function setContent($content, $normalize = true) {
		$this->_body = $normalize ? Mime::charset($content, null, $this->getHeader('Content-Type', 'charset')) : $content;
	}

	function getContent($normalize = true) {
		$body = 'text/html' == strtolower($this->getHeader('Content-Type')) ? 
			trim(tpl_text_format(html_entity_decode(tpl_strip_html_tags($this->_body)))) : $this->_body;
		return $normalize ? Mime::charset($body, $this->getHeader('Content-Type', 'charset')) : $body;
	}

	function getMessageText($callback = null) {
		return clone $this;
	}
	
	/**
	 * Returns false if not an attachment, the filename otherwise
	 * @return	mixed
	 */
	function isAttachment() {
		if (null !== ($name = $this->getHeader('Content-Disposition', 'filename')) ||
		 	null !== ($name = $this->getHeader('Content-Type', 'name')))
			return $name;
		return parent::isAttachment();
	}

	function getAttachments() {
		$disposition = $this->getHeader('Content-Disposition');
		if (('attachment' == strtolower($disposition) || 'inline' == strtolower($disposition) || null === $disposition) && 
			(null !== ($name = $this->getHeader('Content-Disposition', 'filename')) ||
			 null !== ($name = $this->getHeader('Content-Type', 'name'))))
			return array(array('name' => $name, 'data' => $this->getContent(false)));
		return array();
	}
	
	function parse($chunk, $callback = null) {
		$this->_body = Mime::decode(trim($chunk), $this->getHeader('Content-Transfer-Encoding'));
	}

	function acceptIndexer($indexer) {
		parent::acceptIndexer($indexer);
		$indexer->visitFragment($this->_body);
	}
}

/**
 * Multipart contains other mime parts
 */
class MimeMultipart_2_0_0 extends MimePart {
	/**
	 * @access	protected
	 */
	var $_parts = null;

	/**
	 * @access	protected
	 */
	var $_boundary = null;
	
	function __construct($parts = array()) {
		parent::__construct();
		$this->setHeader('MIME-Version', '1.0');
		$this->setHeader('Content-Type', array('multipart/mixed', 'boundary' => $this->getBoundary()));
		$this->_parts = $parts;
	}

	function __clone() {
		parent::__clone();
		foreach (array_keys($this->_parts) as $key)
			$this->_parts[$key] = clone $this->_parts[$key];
	}
	
	protected function getBoundary() {
		if (null === $this->_boundary)
			$this->_boundary = strtoupper(md5(uniqid('')).md5(uniqid('')));
		return $this->_boundary;
	}
	
	function addHeader($name, $value, $charset = null) {
		if ('content-type' == strtolower($name)) {
			$lower = array_change_key_case((array)$value, CASE_LOWER);
			if (isset($lower['boundary']))
				$this->_boundary = $lower['boundary'];
			else
				$value['boundary'] = $this->getBoundary();
		}
		parent::addHeader($name, $value, $charset);
	}

	function getMessageBody() {
		$message = "  This message is in MIME format.  The first part should be readable text,
  while the remaining parts are likely unreadable without MIME-aware tools.
";

		// Render all message parts
		foreach (array_keys($this->_parts) as $key) {
			$message .= "\r\n--".$this->getBoundary()."\r\n";
			$message .= $this->_parts[$key]->toString();
			$message .= "\r\n";
		}
		
		$message .= "--".$this->getBoundary()."--";

		// Transfer encode message
		return Mime::encode($message, $this->getHeader('Content-Transfer-Encoding'));
	}

	function getContent($normalize = true) {
		// Return only textpart if two text/plain and text/html alternatives
		if ('multipart/alternative' == strtolower($this->getHeader('Content-Type')) && 2 == count($this->_parts)) {
			$textpart = 'text/plain' == strtolower($this->_parts[0]->getHeader('Content-Type')) ? 
				$this->_parts[0] : $this->_parts[1];
			$htmlpart = 'text/plain' == strtolower($this->_parts[0]->getHeader('Content-Type')) ? 
				$this->_parts[1] : $this->_parts[0];

			// Use the HTML body when charset=us-ascii which indicates it's from 
			// Office 365 which has encoding issues with the text/plain part.
			if ('us-ascii' == strtolower($textpart->getHeader('Content-Type', 'charset')))
				return $htmlpart->getContent($normalize);

			return $textpart->getContent($normalize);
		}

		$content = null;
		foreach (array_keys($this->_parts) as $key) {
			if (false === ($name = $this->_parts[$key]->isAttachment()) || 
				'' == $name && 0 === stripos($this->_parts[$key]->getHeader('Content-Type'), 'text/')) {
				if (null != $content)
					$content .= "\r\n";
				$content .= $this->_parts[$key]->getContent($normalize);
			}
		}
		
		return $content;
	}

	function getMessageText($callback = null) {
		$clone = clone $this;
		foreach (array_keys($clone->_parts) as $key) {
			if (false === $clone->_parts[$key]->isAttachment() || (null != $callback && call_user_func($callback, $clone->_parts[$key])))
				$clone->_parts[$key] = $clone->_parts[$key]->getMessageText($callback);
			else
				unset($clone->_parts[$key]);
		}
		return $clone;
	}
	
	function getAttachments() {
		$attachments = array();
		foreach (array_keys($this->_parts) as $key)
			$attachments = array_merge($attachments, $this->_parts[$key]->getAttachments());
		return $attachments;
	}
	
	function getParts() {
		return $this->_parts;
	}
	
	function addPart(MimePart $part) {
		$this->_parts[] = $part;
		$this->_message = null;
	}

	function parse($chunk, $callback = null) {
		if (null === $this->_boundary)
			return false;
		
		$verbatim = null != $callback && call_user_func($callback, $this);
		$parts = preg_split('/\r?\n?--'.preg_quote($this->_boundary, '/').'\r?\n?/', 
			Mime::decode($chunk, $this->getHeader('Content-Transfer-Encoding')));
		array_shift($parts);
		
		for ($i=0; $i<count($parts)-1; $i++) {
			if (null != $parts[$i])
				$this->addPart(Mime::parse($parts[$i], $callback, $verbatim));
		}
	}

	function acceptIndexer($indexer) {
		parent::acceptIndexer($indexer);
		foreach ($this->_parts as $part)
			$part->acceptIndexer($indexer);
	}
}

class MimeTextpart extends MimeTextpart_2_0_0 {
	private $_converted = false;

	function __wakeup() {
		if (!$this->_converted) {
			foreach ((array)$this->_headers as $key => $value) {
				$this->_headers[$key] = array($value);
			}

			$this->_converted = true;
		}
	}
}

class MimeMultipart extends MimeMultipart_2_0_0 {
	private $_converted = false;

	function __wakeup() {
		if (!$this->_converted) {
			foreach ((array)$this->_headers as $key => $value) {
				$this->_headers[$key] = array($value);
			}

			$this->_converted = true;
		}
	}
}

