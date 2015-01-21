/**
 * Context menu <ul>
 */
function ContextMenu(parent) {
	this._node = document.createElement('ul');
	this._node.className = 'ContextMenu';
	this._node.style.visibility = 'hidden';
	this._node.style.left = '-5000px';
	this._node.style.top = '-5000px';
	this._submenus = new Object();

	try {
		if (undefined == parent)
			parent = document.getElementsByTagName('body').item(0);
		parent.appendChild(this._node);
	} catch(e) {}

	if (-1 != navigator.appName.indexOf('Internet Explorer') && 'undefined' == typeof Element) {
		this._iframe = document.createElement('iframe');
		if (0 == new String(window.location).toLowerCase().indexOf('https://'))
			this._iframe.src = '/non_existing_page.html';
		this._iframe.style.display = 'none';
		this._iframe.style.position = 'absolute';
		this._iframe.style.filter='progid:DXImageTransform.Microsoft.Alpha(style=0,opacity=0)';
		this._node.parentNode.insertBefore(this._iframe, this._node);
	}
}

/**
 * <ul> node
 * @access	public
 */
ContextMenu.prototype._node = null;

/**
 * Submenu item map
 * @access	public
 */
ContextMenu.prototype._submenus = null;

/**
 * Currently active submenu
 * @access	public
 */
ContextMenu.prototype._active = null;

/**
 * IE windowed-controls-overlap-layers iframe hack
 * @access	public
 */
ContextMenu.prototype._iframe = null;

/**
 * Appends a menu item
 */
ContextMenu.prototype.appendChild = function(item, id) {
	item._parent = this;
	if (undefined != id)
		this._submenus[id] = item;
	this._node.appendChild(item._node);
}

/**
 * Display this context menu
 *
 * The menu will automatically adjust the coordinates to minimize any
 * area hidden outside the viewport.
 *
 * @param	int	X coordinate
 * @param	int	Y coordinate
 */
ContextMenu.prototype.show = function(x, y) {
	var html = document.getElementsByTagName('html').item(0), _this = this;

	this._node.style.top = 
		y - Math.max(ContextMenu.yAbsolute(this._node.parentNode.parentNode) + 
		y + this._node.clientHeight + 10 - (window.getInnerHeight() + html.scrollTop), 0) + 'px';
	
	if (ContextMenu.xAbsolute(this._node.parentNode.parentNode) + x + this._node.clientWidth > window.getInnerWidth() + html.scrollLeft)
		this._node.style.left = Math.max(x - this._node.clientWidth - 10, 0) + 'px';
	else
		this._node.style.left = x + 'px';

	if (this._iframe) {
		this._iframe.style.top = this._node.style.top;
		this._iframe.style.left = this._node.style.left;
		this._iframe.style.width = this._node.clientWidth + 2 + 'px';
		this._iframe.style.height = this._node.clientHeight + 2 + 'px';
		this._iframe.style.display = 'block';
	}

	this._node.style.visibility = 'visible';
}

/**
 * Hide this menu and any open submenus
 */
ContextMenu.prototype.hide = function() {
	for (key in this._submenus) {
		if (this._submenus[key].hide)
			this._submenus[key].hide();
	}
	this._node.style.visibility = 'hidden';
	if (this._iframe)
		this._iframe.style.display = 'none';
}

/**
 * Calculates the absolute x position of an element
 * @access	private
 * @param	Element
 * @return	integer
 */
ContextMenu.xAbsolute = function(oNode) {
	var pos = oNode.offsetLeft;
	if ('div' == oNode.tagName.toLowerCase() && oNode.scrollLeft)
		pos -= oNode.scrollLeft;
	if (oNode.offsetParent)
		pos += ContextMenu.xAbsolute(oNode.offsetParent);
	return pos;
}

/**
 * Calculates the absolute y position of an element
 * @access	private
 * @param	Element
 * @return	integer
 */
ContextMenu.yAbsolute = function(oNode) {
	var pos = oNode.offsetTop;
	if ('div' == oNode.tagName.toLowerCase() && oNode.scrollTop)
		pos -= oNode.scrollTop;
	if (oNode.offsetParent)
		pos += ContextMenu.yAbsolute(oNode.offsetParent);
	return pos;
}

/**
 * Context menu option <li>
 * @param	string		Text content
 * @param	bool		Item opens a submenu
 * @param	function	Function to run when clicked
 */
function ContextItem(caption, hasSubmenu, onclick) {
	var _this = this;
	this._node = document.createElement('li');
	this._node.innerHTML = caption;

	if (hasSubmenu) {
		this._node.className = 'ContextSubmenuItem';

		var timeout = function() {
			if ('ContextSubmenuItemHover' == _this._node.className) 
				_this.show();
			_this._timeout = null;
		};
		
		var hovertimeout = null;
		
		this._node.attachEvent('onmouseover', function() {
			_this._node.className = 'ContextSubmenuItemHover';
			if (_this._timeout)
				window.clearTimeout(_this._timeout);
			_this._timeout = window.setTimeout(timeout, 200);
			if (!_this._submenu && _this._factory) {
				hovertimeout = window.setTimeout(function() {
					_this._factory(new ContextMenu(_this._node), function(submenu) {_this.setSubmenu(submenu);});
				}, 150);
			}
		});

		this._node.attachEvent('onmouseout', function() {
			_this._node.className = 'ContextSubmenuItem';
			if (_this._timeout) {
				window.clearTimeout(_this._timeout);
				window.clearTimeout(hovertimeout);
				_this._timeout = null;
			}
		});
	}
	else {
		this._node.className = 'ContextItem';
		if (!onclick)
			this._node.style.color = '#666';
		
		this._node.attachEvent('onmouseover', function() {
			_this._node.className = 'ContextItemHover';
			if (_this._timeout)
				window.clearTimeout(_this._timeout);
			if (_this._parent._active)
				_this._timeout = window.setTimeout(function() {_this._parent._active.hide();}, 350);
		});
		
		this._node.attachEvent('onmouseout', function() {
			_this._node.className = 'ContextItem';
			if (_this._timeout) {
				window.clearTimeout(_this._timeout);
				_this._timeout = null;
			}
		});
	}

	if (onclick) {
		this._node.attachEvent('onclick', function(ev) {_this._onclick(); ev.cancelBubble = true; return false;});
		this._node.attachEvent('oncontextmenu', function(ev) {_this._onclick(); ev.cancelBubble = true; return false;});
	}
	else {
		// Trap event to prevent menu from closing
		this._node.attachEvent('onclick', function(ev) {ev.cancelBubble = true; return false;});
		this._node.attachEvent('oncontextmenu', function(ev) {ev.cancelBubble = true; return false;});
	}

	this._onclick = onclick;
}

/**
 * Parent ContextMenu object
 * @access	public
 */
ContextItem.prototype._parent = null;

/**
 * <li> node
 * @access	public
 */
ContextItem.prototype._node = null;

/**
 * Function to run when clicked
 * @access	private
 */
ContextItem.prototype._onclick = null;

/**
 * Function to run to request menu loading
 * @access	private
 */
ContextItem.prototype._factory = null;

/**
 * Submenu if created
 * @access	private
 */
ContextItem.prototype._submenu = null;

/**
 * Display submenu timeout is running
 * @access	private
 */
ContextItem.prototype._timeout = false;

/**
 * Shows the submenu if one exists
 */
ContextItem.prototype.show = function() {
	if (null != this._submenu && this._parent._active != this) {
		if (this._parent._active)
			this._parent._active.hide();
		this._parent._active = this;
		this._submenu.show(this._node.offsetLeft + this._node.parentNode.clientWidth, this._node.offsetTop);
	}
}

/**
 * Hides the submenu if one exists
 */
ContextItem.prototype.hide = function() {
	if (this._timeout) {
		window.clearTimeout(this._timeout);
		this._timeout = null;
	}
	if (null != this._submenu) {
		this._parent._active = null;
		this._submenu.hide();
	}
}

/**
 * Specify the submenu factory
 * @param	function 	Function to call to load submenu
 */
ContextItem.prototype.setFactory = function(factory) {
	this._factory = factory;
	if ('ContextSubmenuItemHover' == this._node.className) {
		var _this = this; 
		this._factory(new ContextMenu(this._node), function(submenu) {_this.setSubmenu(submenu);});
	}
}

/**
 * Specify the submenu
 * @param	ContextMenu	Submenu to use for this item
 */
ContextItem.prototype.setSubmenu = function(submenu) {
	this._submenu = submenu;
	if ('ContextSubmenuItemHover' == this._node.className && !this._timeout)
		this.show();
}

/**
 * Emulate IE methods and properties
 */
if ('undefined' != typeof Element && undefined != Element.prototype.__defineGetter__) {
	Element.prototype.attachEvent = function(sEvent, fpCallback) {
		this.addEventListener(sEvent.substring(2), fpCallback, false);
	}

	Element.prototype.detachEvent = function(sEvent, fpCallback) {
		this.removeEventListener(sEvent.substring(2), fpCallback, false);
	}

	Element.prototype.__defineGetter__(
		'innerText',
		function() {
			return this.textContent;
		}
	);

	Element.prototype.__defineSetter__(
		'innerText',
		function(sText) {
			this.textContent = sText;
		}
	);
}

if ('undefined' != typeof Event && undefined != Event.prototype.__defineSetter__) {
	Event.prototype.__defineSetter__(
		'cancelBubble', 
		function(cancel) { 
			if (cancel) {
				this.stopPropagation();
				this.preventDefault();
			}
		}
	);
}

if (undefined != window.innerWidth) {
	window.getInnerWidth = function() {return window.innerWidth;}
	window.getInnerHeight = function() {return window.innerHeight;}
}
else {
	window.getInnerWidth = function() {return document.getElementsByTagName('html').item(0).offsetWidth;}
	window.getInnerHeight = function() {return document.getElementsByTagName('html').item(0).offsetHeight;}
}
