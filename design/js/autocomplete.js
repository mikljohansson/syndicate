/**
 * Prototype chaining inheritence
 *
 * Child classes will inherit the parents methods and properties as
 * well as access to parents constructor and overridded methods through
 * the parent member.
 *
 * this.parent.constructor(arg1, arg2);
 * this.parent.someOverriddenMethod(arg1);
 *
 * @param	function	The parent to inherit from
 */
Function.prototype.inherits = function(parent) {
	var inner = function() {};
	inner.prototype = parent.prototype;
	this.prototype = new inner();
	this.prototype.parent = parent.prototype;
	this.prototype.constructor = this;
}

/**
 * Auto completion using Ajax/XML-RPC
 *
 * Search callback method as
 *  function mySearchCallback(string query) : array()
 * 
 * Results are returned as
 *  array(
 *    'mikl@example.com' => 'Mikael Johansson <mikl@example.com',
 *    ...
 *  )
 *
 * Where the values are presented as options and the keys will be 
 * filled into the search box when the user selects that option.
 *
 * The autocomplete="off" might need to be set on the input directly
 * in the html code for it to work with Firefox.
 *
 * @access		public
 * @package		synd.core.lib
 *
 * @param		object	Text box to attach to
 * @param		string	XML-RPC endpoint uri to talk to
 * @param		string	Method to call
 * @param		boolean	Submit form when suggestions are clicked
 */
function AutoComplete(input, endpoint, method, submitonclick) {
	this._transport = new JsonTransport(endpoint);
	this._method = method;
	this._submit = submitonclick;
	this._cache = new Object();
	this._input = new AutoCompleteInput(input);
	
	var _this = this;
	this._suggestions = document.createElement('div');
	this._suggestions.className = 'AutoComplete';
	this._suggestions.style.display = 'none';
	this._suggestions.style.position = 'absolute';

	document.getElementsByTagName('body').item(0).appendChild(this._suggestions);
	
	if (-1 != navigator.appName.indexOf('Internet Explorer') && 'undefined' == typeof Element) {
		this._suggestions._iframe = document.createElement('iframe');
		if (0 == new String(window.location).toLowerCase().indexOf('https://'))
			this._suggestions._iframe.src = '/non_existing_page.html';
		this._suggestions._iframe.style.display = 'none';
		this._suggestions._iframe.style.position = 'absolute';
		this._suggestions._iframe.style.filter='progid:DXImageTransform.Microsoft.Alpha(style=0,opacity=0)';
		this._suggestions.parentNode.insertBefore(this._suggestions._iframe, this._suggestions);
	}
	
	this._position();
	window.attachEvent('onload', function() {_this._position();});
	window.attachEvent('onresize', function() {_this._position();});

	input.attachEvent('onblur', function() {_this._hide();});
	input.attachEvent('onkeyup', function(ev) {return _this._callback_onkeyup(ev);});
	input.attachEvent('onkeydown', function(ev) {
		if (ev.keyCode == 38 || ev.keyCode == 40) {
			ev.cancelBubble = true;
			return false;
		}
	});
	
	// Prevent form submit if not submit-on-click and something is selected
	if (!this._submit && input.form) {
		input.attachEvent('onkeypress', function(ev) {
			if (13 == ev.keyCode && !ev.ctrlKey && !ev.altKey && !ev.metaKey && null != _this._selected) {
				ev.cancelBubble = true;
				return false;
			}
		});
	}
}

/**
 * The XMLRPC transport
 * @access	private
 */
AutoComplete.prototype._transport = null;

/**
 * The method to invoke on the endpoint
 * @access	private
 */
AutoComplete.prototype._method = null;

/**
 * Submit form when suggestions are clicked
 * @access	private
 */
AutoComplete.prototype._submit = false;

/**
 * Input area to attach to
 * @access	private
 */
AutoComplete.prototype._input = null;

/**
 * The current request connection
 * @access	private
 */
AutoComplete.prototype._request = null;

/**
 * The list of suggestions
 * @access	private
 */
AutoComplete.prototype._suggestions = null;

/**
 * Currently selected suggestion
 * @access	private
 */
AutoComplete.prototype._selected = null;

/**
 * Delay before search is dispatched following a keypress
 * @access	private
 */
AutoComplete.prototype._timeout = 350;

/**
 * Minimum delay before search is dispatched following a keypress
 * @access	private
 */
AutoComplete.prototype._minimum = 250;

/**
 * Optimal delay between keypress and results
 * @access	private
 */
AutoComplete.prototype._optimal = 500;

/**
 * Currently active dispatch timer
 * @access	private
 */
AutoComplete.prototype._timer = null;

/**
 * Cached suggestion results
 * @access	private
 */
AutoComplete.prototype._cache = null;

/**
 * Previously pressed keycode
 * @access	private
 */
AutoComplete.prototype._keycode = null;

/**
 * @access	private
 */
AutoComplete.prototype._callback_onkeyup = function(ev) {
	if (!ev.ctrlKey && !ev.altKey && !ev.metaKey) {
		switch (ev.keyCode) {
			case 16:	// Shift
			case 17:	// Ctrl
			case 18:	// Alt
			case 20:	// Caps Lock
			case 32:	// Space
			case 33:	// Page Up
			case 34:	// Page Down
			case 35:	// End
			case 36:	// Home
			case 37:	// Left
			case 39:	// Right
				break;

			case 9:		// Tab
			case 13:	// Enter
				this._hide();
				break;

			case 27:	// Esc
				if (ev.keyCode != this._keycode) 
					this._input.reset(true);
				else {
					this._input.reset(false);
					this._input.select();
				}
				this._hide();
				break;

			case 38:	// Up
				if ('none' != this._suggestions.style.display && null != this._selected)
					this._select(this._selected.previousSibling ? this._selected.previousSibling : null, false);
				break;

			case 40:	// Down
				if ('none' != this._suggestions.style.display) {
					if (null != this._selected && this._selected.nextSibling)
						this._select(this._selected.nextSibling, false);
					else 
						this._select(this._suggestions.firstChild, false);
				}
				else if (!this._input.empty())
					this._dispatch(true);
				break;

			case 8:		// Backspace
				this._input.save();
				if (this._input.empty())
					this._hide();
				else {
					this._setTimeout(false);
					this._select(null, false);
				}
				break;

			default:
				this._input.save();
				if (this._input.empty())
					this._hide();
				else
					this._setTimeout(true);
		}
	}
	
	this._keycode = ev.keyCode;
}

/**
 * Register timeout to dispatch search after this._timeout millisecs
 * @param	boolean	Select the first result on result arrival
 */
AutoComplete.prototype._setTimeout = function(select) {
	// Clear running timer
	clearTimeout(this._timer);

	// Abort outstanding request
	if (null != this._request) {
		this._request.abort();
		this._request = null;
	}
	
	var _this = this;
	this._timer = setTimeout(
		function() {_this._dispatch(select);}, 
		this._cache[this._input.getValue()] ? this._optimal : this._timeout);
}

/**
 * Dispatches an asyncronous search to server
 * @param	boolean	Select the first result on result arrival
 */
AutoComplete.prototype._dispatch = function(select) {
	var value = this._input.getValue();
	
	if (this._cache[value]) {
		this._populate(this._cache[value]);
		if (select && this._suggestions.firstChild && 0 == this._suggestions.firstChild._key.indexOf(value))
			this._select(this._suggestions.firstChild, true);
	}
	else {
		var _this = this, ts = new Date().getTime();
		this._request = this._transport.invoke(this._method, new Array(value), function(result) {
			// Mark request completed
			_this._request = null;
			
			// Adjust timeout with roundtrip time
			_this._timeout = Math.min(Math.max(Math.round((
				_this._timeout + _this._optimal - (new Date().getTime() - ts)) / 2), _this._minimum), _this._optimal);
			
			// If same value and focused; populate list and select first option 
			if (value == _this._input.getValue() && _this._keycode) {
				_this._populate(result);
				if (select && _this._suggestions.firstChild && _this._suggestions.firstChild._key && 0 == _this._suggestions.firstChild._key.indexOf(value))
					_this._select(_this._suggestions.firstChild, true);
			}
			
			_this._cache[value] = result;
		})
	}
}

/**
 * Populates the list of suggestions
 * @access	private
 * @param	Object	Hashmap of keys and values
 */
AutoComplete.prototype._populate = function(result) {
	var item = null;

	if ('string' == typeof result) {
		item = this._suggestions.insertBefore(document.createElement('div'), this._suggestions.firstChild);
		item.innerHTML = result;
		item.className = 'AutoCompleteMessage';
		item = item.nextSibling ? item.nextSibling : null;
	}
	else {
		if (this._suggestions.firstChild) {
			if (this._suggestions.firstChild._key)
				item = this._suggestions.firstChild;
			else
				this._suggestions.removeChild(this._suggestions.firstChild);
		}
		
		for (key in result) {
			if ('function' != typeof result[key]) {
				if (null == item) {
					item = this._suggestions.appendChild(document.createElement('div'));
					item.attachEvent('onmousedown', this._newMouseDownHandler(item));
					item.attachEvent('onmouseover', this._newMouseOverHandler(item));
					item.attachEvent('onmouseout', this._newMouseOutHandler(item));
				}

				item._key = key;
				item.innerHTML = result[key];
				item.className = '';
				item = item.nextSibling ? item.nextSibling : null;
			}
		}
	}

	// Remove surplus items from end of list
	if (item) {
		while (item != this._suggestions.lastChild)
			this._suggestions.removeChild(this._suggestions.lastChild);
		this._suggestions.removeChild(item);
	}
	
	this._suggestions.style.display = this._suggestions.firstChild ? 'block' : 'none';

	if (this._suggestions._iframe) {
		this._suggestions._iframe.style.height = this._suggestions.clientHeight + 2 + 'px';
		this._suggestions._iframe.style.display = this._suggestions.firstChild ? 'block' : 'none';
	}
}

/**
 * @access	private
 */
AutoComplete.prototype._newMouseDownHandler = function(item) {
	var _this = this;
	return function() {
		_this._hide();
		_this._input.setValue(item._key);
		if (_this._submit)
			_this._input.submit();
	};
}

/**
 * @access	private
 */
AutoComplete.prototype._newMouseOverHandler = function(item) {
	var _this = this;
	return function() {
		if (null != _this._selected && _this._selected != item)
			_this._selected.className = '';
		item.className = 'Selected';
		_this._selected = item;
	};
}

/**
 * @access	private
 */
AutoComplete.prototype._newMouseOutHandler = function(item) {
	var _this = this;
	return function() {
		item.className = '';
		_this._selected = null;
	};
}

/**
 * Select a suggestion
 * @param	Element	The option to select
 * @param	boolean	Select/highlight the autocompleted part of the text
 */
AutoComplete.prototype._select = function(option, highlight, force) {
	if (!option || option._key) {
		if (null != this._selected && this._selected._key)
			this._selected.className = '';
		this._selected = option;

		if (null == this._selected) 
			this._input.reset(true);
		else {
			this._selected.className = 'Selected';
			if (highlight)
				this._input.suggest(this._selected._key);
			else
				this._input.setValue(this._selected._key);
		}
	}
}

/**
 * Hides the suggestions
 * @access	private
 */
AutoComplete.prototype._hide = function() {
	clearTimeout(this._timer);

	this._suggestions.style.display = 'none';
	if (this._suggestions._iframe)
		this._suggestions._iframe.style.display = 'none';

	if (null != this._selected)
		this._selected.className = '';
	this._selected = null;
	this._keycode = null;
}

/**
 * Adjusts the position and width of the list
 * @access	private
 */
AutoComplete.prototype._position = function() {
	this._suggestions.style.top = AutoComplete.yAbsolute(this._input._node) + this._input._node.offsetHeight + 'px';
	this._suggestions.style.left = AutoComplete.xAbsolute(this._input._node) + 'px';
	
	if (this._input._node.offsetWidth) {
		this._suggestions.style.width = this._input._node.offsetWidth - 2 + 'px';
		if (this._suggestions._iframe) {
			this._suggestions._iframe.style.top = this._suggestions.style.top;
			this._suggestions._iframe.style.left = this._suggestions.style.left;
			this._suggestions._iframe.style.width = this._suggestions.style.posWidth + 2 + 'px';
		}
	}
}

/**
 * Calculates the absolute x position of an element
 * @access	private
 * @param	Element
 * @return	integer
 */
AutoComplete.xAbsolute = function(node) {
	var pos = node.offsetLeft;
	if ('div' == node.tagName.toLowerCase() && node.scrollLeft)
		pos -= node.scrollLeft;
	if (node.offsetParent)
		pos += AutoComplete.xAbsolute(node.offsetParent);
	return pos;
}

/**
 * Calculates the absolute y position of an element
 * @access	private
 * @param	Element
 * @return	integer
 */
AutoComplete.yAbsolute = function(node) {
	var pos = node.offsetTop;
	if ('div' == node.tagName.toLowerCase() && node.scrollTop)
		pos -= node.scrollTop;
	if (node.offsetParent)
		pos += AutoComplete.yAbsolute(node.offsetParent);
	return pos;
}

/**
 * Standard input box
 */
function AutoCompleteInput(node) {
	this._node = node;
	this._node.setAttribute('autocomplete', 'off');
}

/**
 * <input> element
 * @access	public
 */
AutoCompleteInput.prototype._node = null;

/**
 * Value typed into input
 * @access	private
 */
AutoCompleteInput.prototype._value = null;

/** 
 * Input box is empty?
 */
AutoCompleteInput.prototype.empty = function() {
	return !this._node.value.length;
}

/** 
 * Current input box value
 */
AutoCompleteInput.prototype.getValue = function() {
	return this._node.value;
}

/** 
 * Displays a value
 */
AutoCompleteInput.prototype.setValue = function(value) {
	this._node.value = value;
	this._node.focus();
}

/**
 * Autosuggest a value
 */
AutoCompleteInput.prototype.suggest = function(value) {
	if (0 == value.indexOf(this._value)) {
		this._node.value = value;
		if (this._node.setSelectionRange) 
			this._node.setSelectionRange(this._value.length, this._node.value.length);
		else if (this._node.createTextRange) {
			range = this._node.createTextRange();
			range.moveStart('character', this._value.length);
			range.select();
		}
	}
}

/** 
 * Savepoint the current value
 */
AutoCompleteInput.prototype.save = function() {
	this._value = this._node.value;
}

/** 
 * Resets the value
 * @param	bool	Reset to default value instead of typed
 */
AutoCompleteInput.prototype.reset = function(savepoint) {
	this._node.value = savepoint ? this._value : this._node.defaultValue;
}

/**
 * Highlight the entire text
 */
AutoCompleteInput.prototype.select = function() {
	if (this._node.setSelectionRange) 
		this._node.setSelectionRange(0, this._node.value.length);
	else if (this._node.createTextRange)
		this._node.createTextRange().select();
}

/**
 * Submits the form
 */
AutoCompleteInput.prototype.submit = function() {
	if (this._node.form)
		this._node.form.submit();
}

if (!window.attachEvent) {
	Element.prototype.attachEvent = function(sEvent, fpCallback) {
		this.addEventListener(sEvent.substring(2), fpCallback, false);
	}
	
	Element.prototype.detachEvent = function(sEvent, fpCallback) {
		this.removeEventListener(sEvent.substring(2), fpCallback, false);
	}

	Window.prototype.attachEvent = Element.prototype.attachEvent;
	Window.prototype.detachEvent = Element.prototype.detachEvent;
}

if ('undefined' != typeof Event && undefined != Event.prototype.__defineGetter__) {
	Event.prototype.__defineGetter__(
		'srcElement', 
		function() {
			return this.target;
		}
	);

	Event.prototype.__defineSetter__(
		'cancelBubble', 
		function(bCancel) { 
			if (true == bCancel) {
				this.stopPropagation();
				this.preventDefault();
			}
		}
	);

	KeyboardEvent.prototype.__defineSetter__(
		'cancelBubble', 
		function(bCancel) { 
			if (true == bCancel) {
				this.stopPropagation();
				this.preventDefault();
			}
		}
	);
}

function debug(obj) {
	var s = '';
	for (p in obj) {
		if ('function' != typeof obj[p])
			s += p+":"+obj[p]+"      ";
	}
	alert('' == s ? obj : s);
}
