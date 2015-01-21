/**
 * Http protocol helper
 */
function HttpConnection(endpoint) {
	if (null != endpoint.match(/^\w+:\/\//))
		this._uri = endpoint;
	else {
		var location = new String(window.location);
		if (-1 == location.indexOf('://'))
			this._uri = endpoint;
		else if ('/' == endpoint.substr(0,1)) {
			var end = location.indexOf('/', location.indexOf('://')+3);
			this._uri = -1 != end ? location.substr(0, end) + endpoint : location + endpoint;
		}
		else {
			var end = location.lastIndexOf('/');
			this._uri = -1 != end ? location.substr(0, end) + '/' + endpoint : location + '/' + endpoint;
		}
	}
}

/**
 * @access	private
 */
HttpConnection.prototype._uri = null;

/**
 * GET method request
 * @param	function	Callback to handle result
 * @param	
 */
HttpConnection.prototype.get = function(callback /* = null */) {
	var http = this.factory();

	if (undefined == callback)
		http.open('GET', this._uri, false);
	else {
		http.open('GET', this._uri, true);
		http.onreadystatechange = this.handler(http, callback);
	}

	http.setRequestHeader('Host', this._uri.replace(/^https?:\/{2}([:\[\]\-\w\.]+)\/?.*/, '$1'));
	http.setRequestHeader('User-Agent', 'Synd (http://www.synd.info/, '+navigator.userAgent+')');
	http.send(null);
	
	if (undefined == callback && '2' != new String(http.status).substring(0,1))
		throw new Error('Invalid HTTP response code ('+http.status+')');
	
	return http;
}

/**
 * GET method request
 * @param	string		Post data to send
 * @param	string		Content-Type of post data
 * @param	function	Callback to handle result
 * @param	
 */
HttpConnection.prototype.post = function(content, type /* = 'application/x-www-form-urlencoded' */, callback /* = null */) {
	var http = this.factory();

	if (undefined == callback)
		http.open('POST', this._uri, false);
	else {
		http.open('POST', this._uri, true);
		http.onreadystatechange = this.handler(http, callback);
	}

	http.setRequestHeader('Host', this._uri.replace(/^https?:\/{2}([:\[\]\-\w\.]+)\/?.*/, '$1'));
	http.setRequestHeader('User-Agent', 'Synd (http://www.synd.info/, '+navigator.userAgent+')');
	http.setRequestHeader('Content-Type', undefined != type ? type : 'application/x-www-form-urlencoded');
	http.send(content);
	
	if (undefined == callback && '2' != new String(http.status).substring(0,1))
		throw new Error('Invalid HTTP response code ('+http.status+')');
	
	return http;
}

/**
 * @access	private
 */
HttpConnection.prototype.factory = function() {
	var http = null;
	
	if ('undefined' != typeof XMLHttpRequest)
		http = new XMLHttpRequest();
	else {
		try {
			http = new ActiveXObject('Msxml2.XMLHTTP');
		}
		catch (e) {
			try {
				http = new ActiveXObject('Microsoft.XMLHTTP');
			}
			catch (e) {}
		}
	}
	
	if (null == http)
		throw new Error('Browser does not support the "XMLHttpRequest" object.');
	return http;
}

/**
 * @access	private
 */
HttpConnection.prototype.handler = function(http, callback) {
	var _this = this;
	return function() {
		if (4 == http.readyState) {
			try {
				var status = new String(http.status);
			}
			catch (e) {return;}

			if ('2' == status.substring(0,1)) {
				callback(http);
			}
		}
	}
}
		
/**
 * Stand alone XML-RPC client implementation
 *
 * Tested with Internet Explorer and Mozilla/Firefox. Should work 
 * with other browsers supporting the XMLHttpRequest object such
 * as Opera 7.6+, Konqueror and Safari.
 *
 * @access		public
 * @package		synd.core.module
 * @link		http://www.xmlrpc.com/
 *
 * @param		string	Absolute URI of remote service (http://www.example.com/synd/rpc/xmlrpc/node.issue.123/)
 */
function XmlRpcTransport(endpoint) {
	this._http = new HttpConnection(endpoint);
}

/**
 * Invokes a method on the remote service
 *
 * If a callback is supplied an async call will be made; in this case
 * the method will return null immediatly. When the HTTP request 
 * completes, the callback is invoked with the decoded result value.
 *
 * If no callback is supplied the method will block until the HTTP
 * request completes and the result will be returned.
 *
 * @throws	object		Error
 * @param	string		Method to invoke
 * @param	array		Array of parameters
 * @param	callback	Callback function on the form 'myAsyncCallback(mixed rpcReturnValue)'
 * @return	mixed		Returns method result on syncronous call, XMLHttpRequest object on async call
 */
XmlRpcTransport.prototype.invoke = function(method, args, callback /* = null */) {
	if (undefined == callback)
		return this._decode(this._http.post(this._encode(method, args), 'text/xml'));
	return this._http.post(this._encode(method, args), 'text/xml', function(http) {callback(_this._decode(http));});
}

/**
 * Encodes a method and parameters into an XML-RPC message
 * @access	private
 * @param	string	Method to invoke
 * @param	array	Array of parameters
 * @return	string
 */
XmlRpcTransport.prototype._encode = function(method, args) {
	var message = '<?xml version="1.0"?><methodCall><methodName>'+method+'</methodName><params>';
	if (undefined != args && args.length) {
		for (var i=0; i<args.length; i++) {
			if (undefined != args[i].toXMLRPC)
				message += '<param><value>'+args[i].toXMLRPC()+'</value></param>';
		}
	}
	return message + '</params></methodCall>';
}

/**
 * Decodes an XML-RPC response into a return value
 * @access	private
 * @param	object	IXMLHTTPRequest
 * @return	mixed
 */
XmlRpcTransport.prototype._decode = function(http) {
	var responseXML, response;
	
	try {
		if (null != http.responseXML)
			responseXML = http.responseXML;
		else
			responseXML = this._parse(http.responseText);		
	} 
	catch (e) {
		responseXML = this._parse(http.responseText);
	}
	
	for (var i=0; i<responseXML.childNodes.length; i++) {
		if (1 == responseXML.childNodes.item(i).nodeType) {
			response = responseXML.childNodes.item(i);
			if ('methodResponse' != response.tagName)
				throw new Error('Malformed XML-RPC response (expected "methodResponse", got "'+response.tagName+'")');
			
			for (var j=0; j<response.childNodes.length; j++) {
				child = response.childNodes.item(j);
			
				if (1 == child.nodeType) {
					switch (child.tagName) {
						case 'params':
							for (var k=0, param; k<child.childNodes.length; k++) {
								param = child.childNodes.item(k);
								if (1 == param.nodeType) {
									if ('param' != param.tagName)
										throw new Error('Malformed XML-RPC response (expected "param", got "'+param.tagName+'")');
									
									for (var l=0; l<param.childNodes.length; l++) {
										if (1 == param.childNodes.item(l).nodeType)
											return this._value(param.childNodes.item(l));
									}
								}
							}
							break;

						case 'fault':
						default:
							try {
								var error = this._value(child.getElementsByTagName('value').item(0));
							}
							catch (e) {
								throw new Error('Malformed XML-RPC response (unknown fault)');
							}
							throw new Error(error['faultString']+' ('+error['faultCode']+')');
					}
				}
			}
		}
	}
	
	throw new Error('Malformed XML-RPC response (no "methodResponse" element)');
}

/**
 * Parses an XML document
 * @access	protected
 * @param	string	XML string to parse
 * @return	object	IDOMDocument
 */
XmlRpcTransport.prototype._parse = function(message) {
	if ('undefined' != typeof DOMParser) {
		var parser = new DOMParser();
		return parser.parseFromString(message, 'text/xml');
	}
	else if (-1 != navigator.appName.indexOf('Internet Explorer')) {
		var parser = null;
		try {
			parser = new ActiveXObject('Msxml2.XMLDOM');
		}
		catch (e) {
			parser = new ActiveXObject('Microsoft.XMLDOM');
		}
		
		parser.loadXML(message);
		return parser;
	}
	
	throw new Error('Browser does not support XMLDOM parsing.');
}

/**
 * Decodes a <value> element into the corresponding JavaScript type
 * @access	private
 * @param	object	IDOMElement
 * @return	mixed
 */
XmlRpcTransport.prototype._value = function(value) {
	switch (value.tagName) {
		case 'value':
			for (var i=0; i<value.childNodes.length; i++) {
				if (1 == value.childNodes.item(i).nodeType)
					return this._value(value.childNodes.item(i));
			}
			throw new Error('Malformed XML-RPC response (empty "value" element)');
		
		case 'string':
			var result = ''; 
			for(var i=0; i<value.childNodes.length; i++)
				result += new String(value.childNodes.item(i).nodeValue);
			return result;

		case 'i4':
		case 'int':
		case 'double':
			return undefined != value.firstChild ? new Number(value.firstChild.nodeValue).valueOf() : 0;

		case 'boolean':
			var result = parseInt(value.firstChild.nodeValue);
			return Boolean(isNaN(result) ? 'true' == value.firstChild.nodeValue : result);

		case 'base64':
			return this._base64(value);
		case 'dateTime.iso8601':
			return this._date(value);
		case 'array':
			return this._array(value);
		case 'struct':
			return this._struct(value);
	}
	
	throw new Error('Malformed XML-RPC response (unknown value element "'+value.tagName+'")');
}

/**
 * Decodes a <base64> element into the corresponding string
 * @access	private
 * @param	object	IDOMElement
 * @return	string
 */
XmlRpcTransport.prototype._base64 = function(value) {
	var encoded = ''; 
	for(var i=0; i<value.childNodes.length; i++)
		encoded += new String(value.childNodes.item(i).nodeValue).replace(/[^a-z0-9\+\/=]/gi, '');
	
	// Try using Mozillas builtin codec
	if ('undefined' != typeof atob)
		return atob(encoded);
	
	var bits, result = new Array(Math.ceil(encoded.length / 4));
	var base64 = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=';
	
	for (var i=0; i<encoded.length; i+=4) {
		bits = (base64.indexOf(encoded.charAt(i))   & 0xff) << 18 |
				(base64.indexOf(encoded.charAt(i+1)) & 0xff) << 12 |
				(base64.indexOf(encoded.charAt(i+2)) & 0xff) <<  6 |
				 base64.indexOf(encoded.charAt(i+3)) & 0xff;
		result[i] = String.fromCharCode((bits & 0xff0000) >> 16, (bits & 0xff00) >> 8, bits & 0xff);
	}

	// Make sure padding chars are left out.
	result[result.length-1] = result[result.length-1].substring(0, 
		3 - ((encoded.charCodeAt(i - 2) == 61) ? 2 : (encoded.charCodeAt(i - 1) == 61 ? 1 : 0)));
	return result.join('');
}

/**
 * Decodes a <dateTime.iso8601> element into the corresponding Date
 * @access	private
 * @param	object	IDOMElement
 * @return	object	Date
 */
XmlRpcTransport.prototype._date = function(value) {
	try {
		if(/^(\d{4})-?(\d{2})-?(\d{2})T(\d{2}):?(\d{2}):?(\d{2})/.test(value.firstChild.nodeValue))
			return new Date(Date.UTC(RegExp.$1, RegExp.$2-1, RegExp.$3, RegExp.$4, RegExp.$5, RegExp.$6));
		else
			throw new Error('Malformed XML-RPC response');
	} 
	catch(e) {
		throw new Error('Malformed XML-RPC response');
	}
}

/**
 * Decodes an <array> element into the corresponding Array
 * @access	private
 * @param	object	IDOMElement
 * @return	object	Array
 */
XmlRpcTransport.prototype._array = function(value) {
	var child = null, result = new Array(), element;
	
	for (var i=0; i<value.childNodes.length; i++) {
		child = value.childNodes.item(i);
		
		if (1 == child.nodeType) {
			if ('data' != child.tagName)
				throw new Error('Malformed XML-RPC response (invalid element "'+child.tagName+'")');

			for (var j=0; j<child.childNodes.length; j++) {
				element = child.childNodes.item(j);
				
				if (1 == element.nodeType) {
					if ('value' != element.tagName || undefined == element.firstChild)
						throw new Error('Malformed XML-RPC response (invalid or empty element "'+child.tagName+'")');
					result.push(this._value(element));
				}
			}
		}
	}
	
	return result;
}

/**
 * Decodes a <struct> element into the corresponding Object
 * @access	private
 * @param	object	IDOMElement
 * @return	object	Object
 */
XmlRpcTransport.prototype._struct = function(value) {
	var child = null, result = new Object();
	var element, name, resultValue;
	
	for (var i=0; i<value.childNodes.length; i++) {
		child = value.childNodes.item(i);
		
		if (1 == child.nodeType) {
			if ('member' != child.tagName)
				throw new Error('Malformed XML-RPC response (invalid element "'+child.tagName+'")');

			name = null, resultValue = null;
			for (var j=0; j<child.childNodes.length; j++) {
				element = child.childNodes.item(j);
				
				if (1 == element.nodeType) {
					if (undefined == element.firstChild)
						throw new Error('Malformed XML-RPC response (empty "member" element)');

					switch (element.tagName) {
						case 'name':
							if (null != name)
								throw new Error('Malformed XML-RPC response (more than one "name" element)');
							name = element.firstChild.nodeValue;
							break;

						case 'value':
							if (null != resultValue)
								throw new Error('Malformed XML-RPC response (more than one "value" element)');
							resultValue = this._value(element);
							break;

						default:
							throw new Error('Malformed XML-RPC response (invalid element "'+element.tagName+'")');
					}
				}
			}

			if (null == name || null == resultValue)
				throw new Error('Malformed XML-RPC response ("name" and/or "value" elements missing)');

			result[name] = resultValue;
		}
	}
	
	return result;
}

/**
 * Extends the basic types interfaces with XML-RPC encoding methods
 *
 * Override the base Object.toXMLRPC() to specify a custom encoding 
 * for application specific classes.
 */
String.prototype.toXMLRPC = function() {
	return '<string><![CDATA['+this.replace('/\]\]/g', '] ]')+']]></string>';
}

Boolean.prototype.toXMLRPC = function() {
	if(true == this)
		return '<boolean>1</boolean>';
	return '<boolean>0</boolean>';
}

Number.prototype.toXMLRPC = function() {
	if (this == parseInt(this))
		return '<int>'+this+'</int>';
	else if (this == parseFloat(this))
		return '<double>'+this+'</double>';
	return false.toXMLRPC();
}

Date.prototype.toXMLRPC = function() {
	var pad = function(str, pad) {
		return (pad+str).substring(new String(str).length);
	}
	
	return '<dateTime.iso8601>'
		+ pad(this.getUTCFullYear(), '0000')
		+ pad(this.getUTCMonth()+1, '00')
		+ pad(this.getUTCDate(), '00') + 'T'
		+ pad(this.getUTCHours(), '00') + ':'
		+ pad(this.getUTCMinutes(), '00') + ':'
		+ pad(this.getUTCSeconds(), '00')
		+ '</dateTime.iso8601>';
}

Array.prototype.toXMLRPC = function() {
	var result = '';
	for(var i=0; i<this.length; i++) {
		if (undefined != this[i].toXMLRPC)
			result += '<value>'+this[i].toXMLRPC()+'</value>';
	}
	return '<array><data>'+result+'</data></array>';
}

Object.prototype.toXMLRPC = function() {
	var value = this.valueOf();
	if(value.toXMLRPC != this.toXMLRPC)
		return value.toXMLRPC();

	var result = '';
	for(property in this) {
		if ('function' != typeof value[property] && undefined != value[property].toXMLRPC)
			result += '<member><name>'+property+'</name><value>'+value[property].toXMLRPC()+'</value></member>';
	}
	return '<struct>'+result+'</struct>';
}

/**
 * Transparent object proxy (uses syncronous communication)
 *
 * If an interface is not specified, introspection using
 * (system.listMethods) will be attempted.
 *
 * @access		public
 * @package		synd.core.module
 * @link		http://www.xmlrpc.com/
 *
 * @param	object	RpcTransport
 * @param	array	Instance interface (list of method names)
 */
function TransparentProxy(transport, ifc) {
	var method = '';
	if (undefined == ifc)
		var ifc = transport.invoke('system.listMethods');

	// A function which use closures to create proxy functions
	var createProxy = function(transport, method) {
		return function() {
			return transport.invoke(method, arguments);
		}
	}
	
	for (i=0; i<ifc.length; i++)
		this[ifc[i]] = createProxy(transport, ifc[i]);
}
