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
 *
 * @param		string	Absolute URI of remote service (http://www.example.com/synd/rpc/json/node.issue.123/)
 */
function JsonTransport(endpoint) {
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
JsonTransport.prototype.invoke = function(method, args, callback /* = null */) {
	if (undefined == callback)
		return this._http.post(new Array(new Array(method, args)).toJSONString(), 'text/javascript').responseText.parseJSON()[0];
	return this._http.post(new Array(new Array(method, args)).toJSONString(), 'text/javascript', function(http) {callback(http.responseText.parseJSON()[0]);});
}

/*
    json.js
    2006-12-06

    This file adds these methods to JavaScript:

        array.toJSONString()
        boolean.toJSONString()
        date.toJSONString()
        number.toJSONString()
        object.toJSONString()
        string.toJSONString()
            These methods produce a JSON text from a JavaScript value.
            It must not contain any cyclical references. Illegal values
            will be excluded.

            The default conversion for dates is to an ISO string. You can
            add a toJSONString method to any date object to get a different
            representation.

        string.parseJSON(hook)
            This method parses a JSON text to produce an object or
            array. It can throw a SyntaxError exception.

            The optional hook parameter is a function which can filter and
            transform the results. It receives each of the values, and its
            return value is used instead. If it returns what it received, then
            structure is not modified.

            Example:

            // Parse the text. If it contains any "NaN" strings, replace them
            // with the NaN value. All other values are left alone.

            myData = text.parseJSON(function (value) {
                if (value === 'NaN') {
                    return NaN;
                }
                return value;
            });

    It is expected that these methods will formally become part of the
    JavaScript Programming Language in the Fourth Edition of the
    ECMAScript standard in 2007.
*/
if (!Object.prototype.toJSONString) {
    Array.prototype.toJSONString = function () {
        var a = ['['], b, i, l = this.length, v;

        function p(s) {
            if (b) {
                a.push(',');
            }
            a.push(s);
            b = true;
        }

        for (i = 0; i < l; i += 1) {
            v = this[i];
            switch (typeof v) {
            case 'undefined':
            case 'function':
            case 'unknown':
                break;
            case 'object':
                if (v) {
                    if (typeof v.toJSONString === 'function') {
                        p(v.toJSONString());
                    }
                } else {
                    p("null");
                }
                break;
            default:
                p(v.toJSONString());
            }
        }
        a.push(']');
        return a.join('');
    };

    Boolean.prototype.toJSONString = function () {
        return String(this);
    };

    Date.prototype.toJSONString = function () {

        function f(n) {
            return n < 10 ? '0' + n : n;
        }

        return '"' + this.getFullYear() + '-' +
                f(this.getMonth() + 1) + '-' +
                f(this.getDate()) + 'T' +
                f(this.getHours()) + ':' +
                f(this.getMinutes()) + ':' +
                f(this.getSeconds()) + '"';
    };

    Number.prototype.toJSONString = function () {
        return isFinite(this) ? String(this) : "null";
    };

    Object.prototype.toJSONString = function () {
        var a = ['{'], b, i, v;

        function p(s) {
            if (b) {
                a.push(',');
            }
            a.push(i.toJSONString(), ':', s);
            b = true;
        }

        for (i in this) {
            if (this.hasOwnProperty(i)) {
                v = this[i];
                switch (typeof v) {
                case 'undefined':
                case 'function':
                case 'unknown':
                    break;
                case 'object':
                    if (v) {
                        if (typeof v.toJSONString === 'function') {
                            p(v.toJSONString());
                        }
                    } else {
                        p("null");
                    }
                    break;
                default:
                    p(v.toJSONString());
                }
            }
        }
        a.push('}');
        return a.join('');
    };


    (function (s) {
        var m = {
            '\b': '\\b',
            '\t': '\\t',
            '\n': '\\n',
            '\f': '\\f',
            '\r': '\\r',
            '"' : '\\"',
            '\\': '\\\\'
        };

        s.parseJSON = function (hook) {
            try {
                if (/^("(\\.|[^"\\\n\r])*?"|[,:{}\[\]0-9.\-+Eaeflnr-u \n\r\t])+?$/.
                        test(this)) {
                    var j = eval('(' + this + ')');
                    if (typeof hook === 'function') {
                        function walk(v) {
                            if (v && typeof v === 'object') {
                                for (var i in v) {
                                    if (v.hasOwnProperty(i)) {
                                        v[i] = walk(v[i]);
                                    }
                                }
                            }
                            return hook(v);
                        }
                        return walk(j);
                    }
                    return j;
                }
            } catch (e) {
            }
            throw new SyntaxError("parseJSON");
        };

        s.toJSONString = function () {
            if (/["\\\x00-\x1f]/.test(this)) {
                return '"' + this.replace(/([\x00-\x1f\\"])/g, function(a, b) {
                    var c = m[b];
                    if (c) {
                        return c;
                    }
                    c = b.charCodeAt();
                    return '\\u00' +
                        Math.floor(c / 16).toString(16) +
                        (c % 16).toString(16);
                }) + '"';
            }
            return '"' + this + '"';
        };
    })(String.prototype);
}