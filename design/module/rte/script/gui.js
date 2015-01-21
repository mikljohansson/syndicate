/**
 * SyndRTE gui functions and callbacks.
 *
 * @access		public
 * @package		synd.rte
 */

var srte_goFormatTags = new Object();
srte_goFormatTags['P']  = true;
srte_goFormatTags['DIV']= true;
srte_goFormatTags['H1'] = true;
srte_goFormatTags['H1'] = true;
srte_goFormatTags['H2'] = true;
srte_goFormatTags['H3'] = true;
srte_goFormatTags['H4'] = true;
srte_goFormatTags['H5'] = true;
srte_goFormatTags['H6'] = true;
srte_goFormatTags['PRE']= true;

var srte_goBlockTags = new Object(srte_goFormatTags);
srte_goBlockTags['BLOCKQUOTE']= true;

var srte_goAlignTags = new Object(srte_goFormatTags);
srte_goAlignTags['BLOCKQUOTE']= true;
srte_goAlignTags['TD'] 	  = true;

var srte_goClassTags = new Object();
srte_goClassTags['P']     = true;
srte_goClassTags['DIV']   = true;
srte_goClassTags['SPAN']  = true;
srte_goClassTags['IMG']   = true;
srte_goClassTags['TABLE'] = true;
srte_goClassTags['TD'] 	  = true;

function srte_button_onclick(sBtnId, bBtnState) {
	var oNode, oArea = SyndArea.getActive();
	switch (sBtnId) {
		case 'srte_button_properties':
		case 'srte_button_dom':
		case 'srte_button_image':
		case 'srte_button_save':
			return;
	}

	if (null == oArea)
		return;

	switch (sBtnId) {
		case 'srte_button_bold':
			srte_api_execBold();
			break;
		case 'srte_button_italic':
			srte_api_execItalic();
			break;
		case 'srte_button_underline':
			srte_api_execUnderline();
			break;
		case 'srte_button_outdent':
			srte_callback_formatOutdent();
			break;
		case 'srte_button_indent':
			srte_callback_formatIndent();
			break;
		case 'srte_button_orderedlist':
			srte_api_execInsertOL();
			break;
		case 'srte_button_unorderedlist':
			srte_api_execInsertUL();
			break;
		case 'srte_button_alignleft':
			if (bBtnState) srte_callback_formatAlign(null);
			else srte_callback_formatAlign('left');
			break;
		case 'srte_button_aligncenter':
			if (bBtnState) srte_callback_formatAlign(null);
			else srte_callback_formatAlign('center');
			break;
		case 'srte_button_alignright':
			if (bBtnState) srte_callback_formatAlign(null);
			else srte_callback_formatAlign('right');
			break;
		case 'srte_button_source':
			return;
	}

	SyndEditor.activateNode(SyndRange.getSelection().getParent());
}


// Interface callbacks //////////////////////////
function srte_callback_formatBlock(sBlockTag) {
	// Find/create format node
	var oNode = SyndLib.findParentByTags(SyndEditor.getActiveNode(), srte_goFormatTags);
	if (null == oNode)
		oNode = _srte_core_wrapSel(document.createElement(sBlockTag));
	else {
		if (null != sBlockTag && '' != sBlockTag)
			oNode = srte_core_mergeNode(oNode, document.createElement(sBlockTag));
		else
			oNode = srte_core_inlineNode(oNode);
	}
	SyndEditor.activateNode(oNode);
}

function srte_callback_formatClass(sClass) {
	// Find outermost node with accepable tagName and length
	var oRange = SyndRange.getSelection();
	var oNode = srte_core_getParentByTagsEx(oRange.getParent(),
		srte_goClassTags, oRange.toString().length)

	// Create new tag if none found
	if (null == oNode && !oRange.isEmpty()) {
		oNode = oRange.wrapInside(document.createElement('SPAN'));
		SyndEditor.selectNode(oNode);
	}

	if (null != oNode) {
		// Remove contained spans with same set of classNames
		var oStyle = document.getElementById('srte_component_class');
		var oChildren = oNode.getElementsByTagName('SPAN');
		for (var i=0; i<oChildren.length; i++) {
			if (null != oStyle.options.item('srte_style_'+oChildren.item(i).className))
				srte_core_inlineNode(oChildren.item(i));
		}

		oNode.className = sClass;
		srte_core_normalizeNode(oNode.parentNode);
		SyndEditor.activateNode(oNode);
	}
}

function srte_callback_formatOutdent() {
	var oNode = srte_core_getParentByTag(SyndEditor.getActiveNode(), 'BLOCKQUOTE');

	if (null != oNode) {
		var oTags = new Object();
			oTags['BLOCKQUOTE'] = true;

		var oParent = srte_core_getParentByTagsEx(oNode.parentNode,
			oTags, SyndLib.trim(oNode.innerText).length);

		if (null == oParent) {
			srte_core_insertAfter(document.createElement('BR'), oNode);
			srte_core_insertAfter(document.createElement('BR'), oNode);
		}

		oNode = srte_core_inlineNode(oNode);
		srte_api_selectNodeStart(oNode);
	}
}

function srte_callback_formatIndent() {
	var oNode, oRange = SyndRange.getSelection();
	oRange.expandBlock();
	oNode = oRange.wrapInside(document.createElement('BLOCKQUOTE'));
	srte_api_selectNodeStart(oNode);

	if (undefined != oNode.nextSibling && 'BR' == oNode.nextSibling.tagName)
		oNode.parentNode.removeChild(oNode.nextSibling);
	if (undefined != oNode.nextSibling && 'BR' == oNode.nextSibling.tagName)
		oNode.parentNode.removeChild(oNode.nextSibling);
}

function srte_callback_formatAlign(sAlign) {
	var oNode = srte_core_getSelBlockCreate(srte_goAlignTags, 'DIV');

	if (null != oNode && sAlign != oNode.style.textAlign) {
		oNode.removeAttribute('align');

		if ('' != oNode.parentNode.style.textAlign &&
			true == srte_goBlockTags[oNode.parentNode.tagName])
		{
			srte_core_pullUpNode(oNode);
			srte_api_selectNodeStart(oNode);
		}

		if (null == sAlign || oNode.parentNode.currentStyle.textAlign == sAlign)
			oNode.style.removeAttribute('textAlign');
		else
			oNode.style.textAlign = sAlign;

		srte_core_normalizeNode(oNode);
		SyndEditor.activateNode(oNode);
	}
}
/////////////////////////////////////////////////



// Area behavior ////////////////////////////////
function SyndArea(oNode) {
	if ('TEXTAREA' == oNode.tagName) {
		oArea = document.createElement('DIV');
		oArea.id = oNode.name;
		oArea.className = oNode.className;
		oArea.innerHTML = oNode.value;
		oNode.parentNode.replaceChild(oArea, oNode);
		oNode = oArea;
	}
	
	if ('' == oNode.innerHTML)
		oNode.innerHTML = '&nbsp;';
	this._area = oNode;

	// TODO: This is ugly (allowing the api to reverse the attachEvent order)
	if (SyndArea._reverseAttachOrder) {
		this.attachArea(oNode);
		SyndEditor.attachArea(oNode);
	}
	else {
		SyndEditor.attachArea(oNode);
		this.attachArea(oNode);
	}

	// Enable editing on node
	oNode.contentEditable = true;
	SyndArea._all.push(this);
	
	var fpChangeCallback = function() {
		oNode.detachEvent('onchange', fpChangeCallback);
		oNode.detachEvent('onpaste', fpChangeCallback);
		oNode.detachEvent('oncut', fpChangeCallback);
		SyndArea._changed = true;
	}
	
	oNode.attachEvent('onkeypress', fpChangeCallback);
	oNode.attachEvent('onpaste', fpChangeCallback);
	oNode.attachEvent('oncut', fpChangeCallback);
}

/**
 * @access	private
 */
SyndArea._all = new Array();
SyndArea._active = null;

// TODO: This is ugly (allowing the api to reverse the attachEvent order)
SyndArea._reverseAttachOrder = false;

/**
 * @static
 * @access	private
 */
SyndArea._changed = false;

SyndEditor.attachEvent('onload', function() {
	window.onbeforeunload = function() {
		if (SyndArea._changed) {
			var str = 'If you click OK all unsaved changes will be lost.';
			if (undefined == window.event)
				return str;
			window.event.returnValue = str;
		}
	};
});


/**
 * Returns all areas
 * @static
 */
SyndArea.getAll = function() {
	return SyndArea._all;
}

/**
 * Returns the currently active area
 * @static
 */
SyndArea.getActive = function() {
	return SyndArea._active;
}

/**
 * @access	private
 */
SyndArea.prototype._area = null;
SyndArea.prototype._isHtml = false;

SyndArea.prototype.attachArea = function(oNode) {
	var oThis = this;

	function _callback_onclick(oEvent) {
		SyndArea._active = oThis;
	}

	function _callback_onkeydown_help(oEvent) {
		if (13 == oEvent.keyCode) {
			srte_env_set('srte_return_help', 1);
			oNode.detachEvent('onkeydown', _callback_onkeydown_help);
			alert("Please Note: \n  Pressing Return gives you a new paragraph <p>. If you \n  were expecting a new line <br /> use Shift-Return instead.");
		}
	}
	
	oNode.attachEvent('onclick', _callback_onclick);
	oNode.attachEvent('oncontrolselect', _callback_onclick);
	if (null == srte_env_get('srte_return_help'))
		oNode.attachEvent('onkeydown', _callback_onkeydown_help);
}

SyndArea.prototype.getName = function() {
	return this._area.id;
}

SyndArea.prototype.getNode = function() {
	return this._area;
}

/**
 * Returns the HTML source in this area
 * @return	string
 */
SyndArea.prototype.getHtml = function() {
	if (this._isHtml)
		return this._area.innerText;
	return this._area.innerHTML;
}

SyndArea.prototype.isHtmlMode = function() {
	return this._isHtml;
}

SyndArea.prototype.setHtmlMode = function() {
	if (this._isHtml)
		return;

	var oMatch, sInner = this._area.innerHTML;
	oMatch = new RegExp('&', 'gmi');
	sInner = sInner.replace(oMatch, '&amp;');
	oMatch = new RegExp('<', 'gmi');
	sInner = sInner.replace(oMatch, '&lt;');
	oMatch = new RegExp('>', 'gmi');
	sInner = sInner.replace(oMatch, '&gt;');

	this._area.innerHTML = SyndLib.syntaxHighlight(sInner);
	this._isHtml = true;

	SyndEditor.activateNode(null);
}

SyndArea.prototype.setTextMode = function() {
	if (!this._isHtml)
		return;

	var oMatch, sInner = this._area.innerText;
	oMatch = new RegExp('<\\?', 'gmi');
	sInner = sInner.replace(oMatch, '&lt;?');
	oMatch = new RegExp('\\?>', 'gmi');
	sInner = sInner.replace(oMatch, '?&gt;');

	this._area.innerHTML = sInner;
	this._isHtml = false;

	SyndEditor.activateNode(this._area)
}
/////////////////////////////////////////////////


// Input behavior ///////////////////////////////
function SyndInput(oNode) {
	var oThis = this;

	this._node = oNode;
	this._events = new Object();

	var fpCallback = function(oEvent) {
		if (oEvent.srcElement != oThis._node)
			oThis._node.fireEvent('onblur');
	}
	this._node.attachEvent('onfocus', function() {
			document.getElementsByTagName('html').item(0).attachEvent('onmousedown', fpCallback);
			oThis._lastValue = oThis._node.value;
		});
	this._node.attachEvent('onblur', function() {
			document.getElementsByTagName('html').item(0).detachEvent('onmousedown', fpCallback);
			if (oThis._lastValue != oThis._node.value) {
				oThis._lastValue = oThis._node.value;
				oThis.fireEvent('onchange', oThis._node.value);
			}
		});

	this._node.attachEvent('onkeypress', function(oEvent) {
			if (13 == oEvent.keyCode) {
				oThis._lastValue = oThis._node.value;
				oThis.fireEvent('onchange', oThis._node.value);
			}
		});
}

SyndInput.prototype.attachEvent = function(sEvent, fpFunction) {
	if (undefined == this._events[sEvent])
		this._events[sEvent] = new Array();
	this._events[sEvent].push(fpFunction);
}

SyndInput.prototype.fireEvent = function(sEvent) {
	if (undefined == this._events[sEvent])
		return;
	for (var i=0; i<this._events[sEvent].length; i++)
		this._events[sEvent][i](arguments[1], arguments[2])
}

SyndInput.prototype.setValue = function(sValue) {
	this._lastValue = sValue;
	this._node.value = sValue;
}
/////////////////////////////////////////////////

// Toolbar behavior /////////////////////////////
function SyndToolbar(oNode) {
	if (arguments.length > 0)
		this.__construct(this, oNode);
}

SyndToolbar.prototype._node = null;
SyndToolbar.prototype._isActive = false;

SyndToolbar.prototype.__construct = function(oThis, oNode) {
	oThis._node = oNode;

	oNode.srte_x = 0;
	oNode.srte_y = 0;

	oNode['srte_offset_x'] = oNode.offsetLeft - document.body.scrollLeft;
	oNode['srte_offset_y'] = oNode.offsetTop  - document.body.scrollTop;

	// Create eventhandlers living inside the current closure/context
	function _callback_onmousedown(oEvent) {
		if ('INPUT' == oEvent.srcElement.tagName || 'SELECT' == oEvent.srcElement.tagName)
			return;

		oNode.style.zIndex = 20;
		oNode.srte_x = oNode.offsetLeft - oEvent.clientX;
		oNode.srte_y = oNode.offsetTop - oEvent.clientY;

		oThis.setInactive();
		document.getElementsByTagName('html').item(0).attachEvent('onmousemove', _callback_onmousemove);
		document.getElementsByTagName('html').item(0).attachEvent('onmouseup', _callback_onmouseup);
	}

	function _callback_onmousemove(oEvent) {
		oEvent = oEvent ? oEvent : event;
		oNode.style.left = oEvent.clientX + oNode.srte_x + 'px';
		oNode.style.top  = oEvent.clientY + oNode.srte_y + 'px';
	}

	function _callback_onmouseup() {
		document.getElementsByTagName('html').item(0).detachEvent('onmousemove', _callback_onmousemove);
		document.getElementsByTagName('html').item(0).detachEvent('onmouseup', _callback_onmouseup);
		oNode.detachEvent('ondrag', _callback_onmousemove);
		oNode.style.zIndex = 10;

		srte_env_set(oNode.id+'[x]', oNode.style.left);
		srte_env_set(oNode.id+'[y]', oNode.style.top);
		oNode['srte_offset_x'] = oNode.offsetLeft - document.body.scrollLeft;
		oNode['srte_offset_y'] = oNode.offsetTop  - document.body.scrollTop;

		oThis.setActive();
	}

	function _callback_onscroll() {
		oNode.style.left = document.body.scrollLeft + oNode['srte_offset_x'] + 'px';
		oNode.style.top  = document.body.scrollTop  + oNode['srte_offset_y'] + 'px';
	}

	// Hook up dragging and scrolling
	oNode.attachEvent('onmousedown', _callback_onmousedown);
	window.attachEvent('onscroll', _callback_onscroll);

	oThis.setActive();
	oNode.fireEvent('onpropertychange');
}

SyndToolbar.prototype._callback_onpropertychange = function() {
	if (!this._isActive && 'hidden' != this._node.currentStyle.visibility) {
		this._enable();
		this._isActive = true;
	}
	else if (this._isActive && 'hidden' == this._node.currentStyle.visibility) {
		this._disable();
		this._isActive = false;
	}
}

SyndToolbar.prototype._callback_toggle = function() {
	this._node.style.visibility =
		'visible' == this._node.currentStyle.visibility ? 'hidden' : 'visible';
	this._node.fireEvent('onpropertychange');
	SyndEditor.activateNode(SyndEditor.getActiveNode());
}

SyndToolbar.prototype._enable = function() {
	srte_env_set(this._node.id+'[vis]', 'visible');
}

SyndToolbar.prototype._disable = function() {
	srte_env_set(this._node.id+'[vis]', 'hidden');
}

SyndToolbar.prototype.setActive = function() {
	if (undefined == this._node.srte_is_online || !this._node.srte_is_online) {
		var oThis = this;
		this._node.srte_is_online = true;
		this.__onpropertychange = function() {oThis._callback_onpropertychange()}
		this._node.attachEvent('onpropertychange', this.__onpropertychange);
	}

	var oHtml = document.getElementsByTagName('html').item(0);
	if (this._node.offsetLeft < 0 || this._node.offsetLeft > oHtml.scrollWidth ||
		this._node.offsetTop < 0 || this._node.offsetTop > oHtml.scrollHeight) {
		this._node.style.left = '20px';
		this._node.style.top = '20px';
	}
}

SyndToolbar.prototype.setInactive = function() {
	if (undefined != this._node.srte_is_online && this._node.srte_is_online) {
		this._node.detachEvent('onpropertychange', this.__onpropertychange);
		this._node.srte_is_online = false;
	}

	var oHtml = document.getElementsByTagName('html').item(0);
	if (this._node.offsetLeft < 0 || this._node.offsetLeft > oHtml.scrollWidth ||
		this._node.offsetTop < 0 || this._node.offsetTop > oHtml.scrollHeight) {
		this._node.style.left = '20px';
		this._node.style.top = '20px';
	}
}
/////////////////////////////////////////////////


// Button toolbar ///////////////////////////////
function SyndButtonToolbar(oNode) {
	this.__construct(oNode);
}

SyndButtonToolbar.prototype = new SyndToolbar();
SyndButtonToolbar.prototype.constructor = SyndButtonToolbar;
SyndButtonToolbar.prototype.parent = SyndToolbar.prototype;
SyndButtonToolbar.prototype._lastAlign = null;

SyndButtonToolbar.prototype.__construct = function(oNode) {
	var oThis = this, i;

	this.__onactivate = function(oNode) {oThis._callback_onactivate(oNode)};
	this.parent.__construct.call(this, this, oNode);

	var oAll = oNode.getElementsByTagName('*');
	for (i=0; i<oAll.length; i++)
		oAll.item(i).UNSELECTABLE = 'on';
		
	// Setup the insert image toolbar
	var oImageToolbar = document.getElementById('srte_toolbar_image');
	this._tbImage = new SyndToolbar(oImageToolbar);

	if (null != oImageToolbar) {
		oAll = oImageToolbar.getElementsByTagName('*');
		for (i=0; i<oAll.length; i++) {
			if ('INPUT' != oAll.item(i).tagName)
				oAll.item(i).UNSELECTABLE = 'on';
		}

		this._tbImage._node.attachEvent('onpropertychange', function() {
				if ('visible' == oThis._tbImage._node.style.visibility)
					oThis._btnImage.setPressedState();
				else
					oThis._btnImage.setDefaultState();
			});
	}

	// Setup the image toolbar iframe callback
	this._tbImage._frame = document.getElementById('srte_component_image');
	this._tbImage._frame._src = this._tbImage._frame.src;
	
	// Setup the image toolbar iframe callback
	this._tbImage._frame = document.getElementById('srte_component_image');
	this._tbImage._frame._callback_oninsert = function(sImageUri) {
		var oImage = document.createElement('IMG');
		oImage.src = sImageUri;
		if (null != SyndEditor.getLastSelection())
			SyndEditor.getLastSelection().replaceNode(oImage);
		oThis._tbImage._node.style.visibility = 'hidden';
	}
	
	// Instantiate the regular buttons
	if (null != (this._btnSave = SyndButton(document.getElementById('srte_button_save'))))
		this._btnSave.attachEvent('onclick', SyndEditor.saveDocument);
	

	this._btnBold = SyndButton(document.getElementById('srte_button_bold'));
	this._btnItalic = SyndButton(document.getElementById('srte_button_italic'));
	this._btnUnderline = SyndButton(document.getElementById('srte_button_underline'));

	this._btnUnorderedList = SyndButton(document.getElementById('srte_button_unorderedlist'));
	this._btnOrderedList = SyndButton(document.getElementById('srte_button_orderedlist'));
	this._btnOutdent = SyndButton(document.getElementById('srte_button_outdent'));
	this._btnIndent = SyndButton(document.getElementById('srte_button_indent'));

	this._btnAlignLeft = SyndButton(document.getElementById('srte_button_alignleft'));
	this._btnAlignCenter = SyndButton(document.getElementById('srte_button_aligncenter'));
	this._btnAlignRight = SyndButton(document.getElementById('srte_button_alignright'));

	if (null != (this._btnTable = SyndButton(document.getElementById('srte_button_table'))))
		this._btnTable.attachEvent('onclick', function() {oThis._callback_button_table()});
	if (null != (this._btnSource = SyndButton(document.getElementById('srte_button_source'))))
		this._btnSource.attachEvent('onclick', function() {oThis._callback_button_source()});
	if (null != (this._btnImage = SyndButton(document.getElementById('srte_button_image'))))
		this._btnImage.attachEvent('onclick', function() {oThis._callback_button_image()});
}

SyndButtonToolbar.prototype._enable = function() {
	this.parent._enable.call(this);
	SyndEditor.attachEvent('onactivate', this.__onactivate);
}

SyndButtonToolbar.prototype._disable = function() {
	this.parent._disable.call(this);
	SyndEditor.detachEvent('onactivate', this.__onactivate);
}

SyndButtonToolbar.prototype._callback_onactivate = function(oNode) {
	var oArea = SyndArea.getActive();
	if (null == oArea)
		this._btnSource.setDisabledState();
	else if (oArea.isHtmlMode())
		this._btnSource.setPressedState();
	else
		this._btnSource.setDefaultState();

	if (null == oNode || null == oArea || oArea.isHtmlMode()) {
		this._btnSave.setDisabledState();

		this._btnBold.setDisabledState();
		this._btnItalic.setDisabledState();
		this._btnUnderline.setDisabledState();

		this._btnUnorderedList.setDisabledState();
		this._btnOrderedList.setDisabledState();
		this._btnOutdent.setDisabledState();
		this._btnIndent.setDisabledState();

		this._btnAlignLeft.setDisabledState();
		this._btnAlignCenter.setDisabledState();
		this._btnAlignRight.setDisabledState();

		this._btnTable.setDisabledState();
		this._btnImage.setDisabledState();
	}
	else {
		this._btnSave.setDefaultState();
		this._btnOutdent.setDefaultState();
		this._btnIndent.setDefaultState();

		this._btnTable.setDefaultState();
		this._btnImage.setDefaultState();

		// Build hashmap of tagnames in the current branch
		var oBranch = new Object(), oCurrentNode = oNode;
		while (null != oCurrentNode && 'srte_area' != oCurrentNode.className) {
			oBranch[oCurrentNode.tagName] = true;
			oCurrentNode = oCurrentNode.parentNode;
		}

		if (true == oBranch['STRONG'] || true == oBranch['B'] || '700' == oNode.currentStyle.fontWeight)
			this._btnBold.setPressedState();
		else
			this._btnBold.setDefaultState();

		if (true == oBranch['EM'] || 'italic' == oNode.currentStyle.fontStyle)
			this._btnItalic.setPressedState();
		else
			this._btnItalic.setDefaultState();

		if (true == oBranch['U'] || 'underline' == oNode.currentStyle.textDecoration)
			this._btnUnderline.setPressedState();
		else
			this._btnUnderline.setDefaultState();

		switch (oNode.currentStyle.textAlign.toLowerCase()) {
			case 'center':
				this._btnAlignLeft.setDefaultState();
				this._btnAlignCenter.setPressedState();
				this._btnAlignRight.setDefaultState();
				break;
			case 'right':
				this._btnAlignLeft.setDefaultState();
				this._btnAlignCenter.setDefaultState();
				this._btnAlignRight.setPressedState();
				break;
			default:
				this._btnAlignLeft.setPressedState();
				this._btnAlignCenter.setDefaultState();
				this._btnAlignRight.setDefaultState();
		}

		if (true == oBranch['UL'])
			this._btnUnorderedList.setPressedState();
		else
			this._btnUnorderedList.setDefaultState();

		if (true == oBranch['OL'])
			this._btnOrderedList.setPressedState();
		else
			this._btnOrderedList.setDefaultState();
	}
}

SyndButtonToolbar.prototype._callback_button_table = function() {
	if (null == SyndEditor.getActiveNode())
		return;

	var oRow = document.createElement('TR');
	oRow.appendChild(SyndTable.createCell());
	oRow.appendChild(SyndTable.createCell());

	var oBody = document.createElement('TBODY');
	oBody.appendChild(oRow);

	var oTable = document.createElement('TABLE');
	oTable.appendChild(oBody);

	SyndRange.getSelection().replaceNode(oTable);
}

SyndButtonToolbar.prototype._callback_button_source = function() {
	var oArea = SyndArea.getActive();
	if (null != oArea) {
		if (oArea.isHtmlMode())
			oArea.setTextMode();
		else
			oArea.setHtmlMode();
	}
}

SyndButtonToolbar.prototype._callback_button_image = function() {
	if ('visible' == this._tbImage._node.style.visibility)
		this._tbImage._node.style.visibility = 'hidden';
	else {
		var sAction, oArea = SyndArea.getActive();
		var oForm = this._tbImage._frame.contentWindow.document.getElementById('srte_image_form');
		
		if (null != oArea && null != oForm && 
			null != (sAction = oArea.getNode().getAttribute('upload'))) {
			oForm.action = sAction + (-1 == sAction.indexOf('?') ? '?' : '&') + 
				'redirect='+this._tbImage._frame._src;
			this._tbImage._node.style.visibility = 'visible';
		}
	}
}
/////////////////////////////////////////////////


// Properties toolbar ///////////////////////////
function SyndPropertiesToolbar(oNode) {
	this.__construct(oNode);
}

SyndPropertiesToolbar.prototype = new SyndToolbar();
SyndPropertiesToolbar.prototype.constructor = SyndPropertiesToolbar;
SyndPropertiesToolbar.prototype.parent = SyndToolbar.prototype;

SyndPropertiesToolbar.prototype._text = null;
SyndPropertiesToolbar.prototype._table = null;
SyndPropertiesToolbar.prototype._image = null;

SyndPropertiesToolbar.prototype._format = null;
SyndPropertiesToolbar.prototype._class = null;
SyndPropertiesToolbar.prototype._link = null;

SyndPropertiesToolbar.prototype.__construct = function(oNode) {
	var oThis = this;

	this._text = document.getElementById('srte_properties_text');
	this._table = new SyndTableProperties(document.getElementById('srte_properties_table'));
	this._image = new SyndImageProperties(document.getElementById('srte_properties_image'));

	// Format select box
	this._format = document.getElementById('srte_component_format');
	if (null != this._format) {
		this._format.attachEvent('onchange', function() {
			srte_callback_formatBlock(oThis._format.options[oThis._format.selectedIndex].value);
		});
	}

	// Class select box
	this._class = document.getElementById('srte_component_class');
	if (null != this._class) {
		this._class.attachEvent('onchange', function() {
			srte_callback_formatClass(oThis._class.options[oThis._class.selectedIndex].value);
		});
	}
	
 	this._link = document.getElementById('srte_component_link');

	this.__onactivate = function(oNode) {oThis._callback_onactivate(oNode)}
	this.parent.__construct.call(this, this, oNode);

	this._btnProperties = SyndButton(document.getElementById('srte_button_properties'));
	if ('visible' == oNode.currentStyle.visibility)
		this._btnProperties.setPressedState();
	else
		this._btnProperties.setDefaultState();

	// Hook up buttons
	if (null != (oButton = document.getElementById('srte_button_properties')))
		oButton.attachEvent('onclick', function() {oThis._callback_toggle()});
	if (null != (oButton = document.getElementById('srte_toolbar_properties_close')))
		oButton.attachEvent('onclick', function() {oThis._callback_toggle()});

	var oAll = oNode.getElementsByTagName('*');
	for (var i=0; i<oAll.length; i++) {
		if ('INPUT' != oAll.item(i).tagName)
			oAll.item(i).UNSELECTABLE = 'on';
	}

	// Hook up the link text input
	if (null != this._link) {
		var oLinkInput = new SyndInput(this._link);
		oLinkInput.attachEvent('onchange', function() {oThis._callback_link_onblur()});
	}
}

SyndPropertiesToolbar.prototype._enable = function() {
	this.parent._enable.call(this);
	// TODO: Why don't I use NullButton?
	if (null != this._btnProperties)
		this._btnProperties.setPressedState();
	SyndEditor.attachEvent('onactivate', this.__onactivate);
}

SyndPropertiesToolbar.prototype._disable = function() {
	this.parent._disable.call(this);
	this._btnProperties.setDefaultState();
	SyndEditor.detachEvent('onactivate', this.__onactivate);
}

SyndPropertiesToolbar.prototype._callback_onactivate = function(oNode) {
	if (null != oNode && 'TABLE' == oNode.tagName) {
		this._text.style.display = 'none';
		this._image.hide();
		this._table.show(oNode);
	}
	else if (null != oNode && 'IMG' == oNode.tagName) {
		this._text.style.display = 'none';
		this._table.hide();
		this._image.show(oNode);
	}
	else {
		this._table.hide();
		this._image.hide();
		
		// IE doesn't accept display='table'
		this._text.style.display = 'block';

		var sLink = null, iFormatIndex = 0, sTagName, iStyleIndex = 0, i;
		while (null != oNode && 'srte_area' != oNode.className) {
			// Update format selector
			if (0 == iFormatIndex && true == srte_goFormatTags[oNode.tagName]) {
				for (i=0; i<this._format.options.length; i++) {
					if (this._format.options[i].value == oNode.tagName) {
						iFormatIndex = i;
						break;
					}
				}
			}

			// Update class selector
			if (0 == iStyleIndex && '' != oNode.className) {
				for (i=0; i<this._class.options.length; i++) {
					if (this._class.options[i].value == oNode.className) {
						iStyleIndex = i;
						break;
					}
				}
			}

			// Update link selector
			if (null == sLink && oNode.getAttribute && 'IMG' != oNode.tagName)
				sLink = oNode.getAttribute('href');

			oNode = oNode.parentNode;
		}

		this._link.value = null != sLink ? sLink : '';
		this._format.selectedIndex = iFormatIndex;
		this._class.selectedIndex = iStyleIndex;
	}
}

SyndPropertiesToolbar.prototype._callback_link_onblur = function() {
	var oRange = SyndEditor.getLastSelection();
	if (null == oRange)
		return;
	
	if (!oRange.isEmpty())
		oRange.createLink(this._link.value);
	else {
		var oLink = srte_core_getParentByTag(oRange.getParent(), 'A');
		if (null != oLink)
			oLink.href = this._link.value;
		else {
			oRange.expandWord();
			oRange.createLink(this._link.value);
		}
	}
}

function SyndImageProperties(oNode) {
	var oThis = this;
	this._node = oNode;

	// Image refresh handler
	this._callback_onpropertychange = function() {
		SyndEditor.activateNode(oThis._image);}

	// Property input controls
	this._width = new SyndInput(document.getElementById('srte_properties_image_width'));
	this._height = new SyndInput(document.getElementById('srte_properties_image_height'));
	this._link = new SyndInput(document.getElementById('srte_properties_image_link'));
	this._src = new SyndInput(document.getElementById('srte_properties_image_src'));
	this._alt = new SyndInput(document.getElementById('srte_properties_image_alt'));
	this._border = new SyndInput(document.getElementById('srte_properties_image_border'));

	// Property change handlers
	this._width.attachEvent('onchange', function(sValue) {
			oThis._image.width = sValue;
			if (undefined != oThis._image.style.width)
				oThis._image.style.width = '';
		});
	this._height.attachEvent('onchange', function(sValue) {
			oThis._image.height = sValue;
			if (undefined != oThis._image.style.height)
				oThis._image.style.height = '';
		});
	this._link.attachEvent('onchange', function(sValue) {
			var oLink = srte_core_getParentByTag(oThis._image, 'A');
			if (null != oLink) {
				if ('' == sValue)
					srte_core_inlineNode(oLink);
				else
					oLink.href = sValue;
			}
			else {
				oLink = document.createElement('A');
				oLink.href = sValue;
				oLink.appendChild(oThis._image.parentNode.replaceChild(oLink, oThis._image));
			}
		});
	this._src.attachEvent('onchange', function(sValue) {
			if ('' != sValue)
				oThis._image.src = sValue;
		});
	this._alt.attachEvent('onchange', function(sValue) {
			if ('' == sValue)
				oThis._image.removeAttribute('alt');
			else
				oThis._image.alt = sValue;
		});
	this._border.attachEvent('onchange', function(sValue) {
			if ('' == sValue)
				oThis._image.removeAttribute('border');
			else
				oThis._image.border = sValue;
		});
}

SyndImageProperties.prototype.show = function(oImage) {
	this._node.style.display = 'block';

	if (null == this._image || this._image != oImage) {
		// Detach refresh handler from previous image
		if (null != this._image)
			this._image.detachEvent('onpropertychange', this._callback_onpropertychange);
		this._image = oImage;
		this._image.attachEvent('onpropertychange', this._callback_onpropertychange);
	}

	// Update properties
	this._width.setValue(null != this._image.width ? 
		this._image.width : this._image.style.width);
	this._height.setValue(null != this._image.height ? 
		this._image.height : this._image.style.height);

	var oLink = srte_core_getParentByTag(this._image, 'A');
	this._link.setValue(null != oLink ? oLink.href : '');

	this._src.setValue(this._image.src)
	this._alt.setValue(this._image.alt)
	this._border.setValue(this._image.border)
}

SyndImageProperties.prototype.hide = function() {
	this._node.style.display = 'none';
	if (null != this._image) {
		this._image.detachEvent('onpropertychange', this._callback_onpropertychange);
		this._image = null;
	}
}

function SyndTableProperties(oNode) {
	var oThis = this;
	this._node = oNode;
	this._table = null;

	// Table refresh handler
	this._callback_onpropertychange = function() {
		SyndEditor.activateNode(oThis._table.getNode());}

	// Property input controls
	this._rows = new SyndInput(document.getElementById('srte_properties_table_rows'));
	this._cols = new SyndInput(document.getElementById('srte_properties_table_cols'));
	this._width = new SyndInput(document.getElementById('srte_properties_table_width'));
	this._height = new SyndInput(document.getElementById('srte_properties_table_height'));
	this._cellpadding = new SyndInput(document.getElementById('srte_properties_table_cellpadding'));
	this._cellspacing = new SyndInput(document.getElementById('srte_properties_table_cellspacing'));
	this._border = new SyndInput(document.getElementById('srte_properties_table_border'));

	// Property change handlers
	this._rows.attachEvent('onchange', function(sValue) {
			oThis._table.setRowCount(sValue);
		});
	this._cols.attachEvent('onchange', function(sValue) {
			oThis._table.setColumnCount(sValue);
		});
	this._width.attachEvent('onchange', function(sValue) {
			oThis._table.setWidth(sValue);
		});
	this._height.attachEvent('onchange', function(sValue) {
			oThis._table.setHeight(sValue);
		});
	this._cellpadding.attachEvent('onchange', function(sValue) {
			oThis._table.setCellPadding(sValue);
		});
	this._cellspacing.attachEvent('onchange', function(sValue) {
			oThis._table.setCellSpacing(sValue);
		});
	this._border.attachEvent('onchange', function(sValue) {
			oThis._table.setBorderWidth(sValue);
		});
}

SyndTableProperties.prototype.show = function(oTable) {
	this._node.style.display = 'block';

	if (null == this._table || this._table.getNode() != oTable) {
		// Detach refresh handler from previous table
		if (null != this._table)
			this._table.getNode().detachEvent('onpropertychange', this._callback_onpropertychange);

		// New table controller and attach refresh handler
		this._table = SyndTable.factory(oTable);
		oTable.attachEvent('onpropertychange', this._callback_onpropertychange);
	}

	// Update properties
	this._rows.setValue(this._table.getRowCount());
	this._cols.setValue(this._table.getColumnCount());
	this._width.setValue(this._table.getWidth());
	this._height.setValue(this._table.getHeight());
	this._cellpadding.setValue(this._table.getCellPadding());
	this._cellspacing.setValue(this._table.getCellSpacing())
	this._border.setValue(this._table.getBorderWidth())
}

SyndTableProperties.prototype.hide = function() {
	this._node.style.display = 'none';
	if (null != this._table) {
		this._table.getNode().detachEvent('onpropertychange', this._callback_onpropertychange);
		this._table = null;
	}
}
/////////////////////////////////////////////////


// DOM toolbar //////////////////////////////////
function SyndDOMToolbar(oNode) {
	var oThis = this;
	if (null == (this._browser = document.getElementById('srte_component_dombrowser')))
		return;

	this.__onactivate = function(oNode) {oThis._callback_onactivate(oNode)}
	this.__construct(this, oNode);

	// Hook up buttons
	this._btnDOM = SyndButton(document.getElementById('srte_button_dom'));
	this._btnDOM.attachEvent('onclick', function() {oThis._callback_toggle()});
	if ('visible' == oNode.currentStyle.visibility)
		this._btnDOM.setPressedState();
	else
		this._btnDOM.setDefaultState();

	if (null != (oButton = document.getElementById('srte_toolbar_dom_close')))
		oButton.attachEvent('onclick', function() {oThis._callback_toggle()});
}

SyndDOMToolbar.prototype = new SyndToolbar();
SyndDOMToolbar.prototype.constructor = SyndDOMToolbar;
SyndDOMToolbar.prototype.parent = SyndToolbar.prototype;
SyndDOMToolbar.prototype._browser = null;

SyndDOMToolbar.prototype._enable = function() {
	this.parent._enable.call(this);
	if (null != this._btnDOM)
		this._btnDOM.setPressedState();
	SyndEditor.attachEvent('onactivate', this.__onactivate);
}

SyndDOMToolbar.prototype._disable = function() {
	this.parent._disable.call(this);
	this._btnDOM.setDefaultState();
	SyndEditor.detachEvent('onactivate', this.__onactivate);
}

SyndDOMToolbar.prototype._callback_onactivate = function(oNode) {
	var oDomNode = null;
	var bFullSelect = false;
	this._browser.innerHTML = '';

	if (null != oNode && null != SyndArea.getActive() && !SyndArea.getActive().isHtmlMode()) {
		var iLength = SyndRange.getSelection().toString().length;
		if (oNode.innerText.length == iLength)
			bFullSelect = true;

		while (null != oNode && 'srte_area' != oNode.className) {
			oDomNode = this._createDomNode(oNode, bFullSelect);
			this._browser.insertBefore(oDomNode, this._browser.firstChild);
			bFullSelect = false;
			oNode = oNode.parentNode;
		}
	}
}

SyndDOMToolbar.prototype._createDomNode = function(oNode, bSelected) {
	var oDomNode, sDomNodeText;

	sDomNodeText = '> ';

	if ('' != oNode.id)
		sDomNodeText = '#'+oNode.id+sDomNodeText;
	if (oNode.style && '' != oNode.style.textAlign)
		sDomNodeText = '.'+oNode.style.textAlign+sDomNodeText;
	if ('' != oNode.className)
		sDomNodeText = '.'+oNode.className+sDomNodeText;

	sDomNodeText = '<'+oNode.tagName.toLowerCase()+sDomNodeText;

	oDomNode = document.createElement('A');
	oDomNode.href		  = '#';
	oDomNode.innerText 	  = sDomNodeText;
	oDomNode.srte_element = oNode;

	if (true == bSelected)
		oDomNode.className = 'srte_component_dombrowsernode srte_highlight';
	else
		oDomNode.className = 'srte_component_dombrowsernode';

	var fpCallback = function(oEvent) {
		if (null != oEvent.srcElement)
			SyndEditor.selectNode(oEvent.srcElement.srte_element);
	}

	oDomNode.attachEvent('onclick', fpCallback);
	oDomNode.attachEvent('ondblclick', fpCallback);

	oDomNode.attachEvent('oncontextmenu', function(oEvent) {
		return SyndEditor.contextMenu(oEvent, oNode);});

	return oDomNode;
}
/////////////////////////////////////////////////




// Button behavior //////////////////////////////
function SyndButton(oNode) {
	if (null == oNode)
		return SyndButton.nullInstance();

	// Add child class functionality to node
	oNode.setDisabledState = function() {
		if (0 != oNode._state) {
			oNode._state = 0;
			oNode.fireEvent('onmouseout');
		}
	}

	oNode.setDefaultState = function() {
		if (1 != oNode._state) {
			oNode._state = 1;
			oNode.fireEvent('onmouseout');
		}
	}

	oNode.setPressedState = function() {
		if (2 != oNode._state) {
			oNode._state = 2;
			oNode.fireEvent('onmouseout');
		}
	}

	oNode.isNull = function() {
		return false;
	}

	// Create eventhandlers living inside the current closure/context
	function _callback_onclick() {
		if (0 != oNode._state) {
			srte_button_onclick(oNode.id, oNode.srte_button_state);
			oNode.fireEvent('onmouseover');
		}
	}

	function _callback_onmouseover() {
		if (0 == oNode._state)
			return;

		oNode.style.border = "1px solid #0A246A";
		if (1 == oNode._state) {
			oNode.style.backgroundColor = "#b9c0d5";
			oNode.firstChild.style.margin = "0px 2px 2px 0px";
			if (oNode.firstChild.filters)
				oNode.firstChild.filters.item("DropShadow").color = "#8c8c8c";
		}
		else if (2 == oNode._state) {
			oNode.style.backgroundColor = "#a0a7bc";
			oNode.firstChild.style.margin = "1px";
			if (oNode.firstChild.filters)
				oNode.firstChild.filters.item("DropShadow").color = oNode.style.backgroundColor;
		}
	}

	function _callback_onmouseout() {
		switch (oNode._state) {
			case 2:
				oNode.style.border = "1px solid #0A246A";
				oNode.style.backgroundColor = "#d3d6dd";
				break;
			case 1:
			case 0:
				oNode.style.border = "1px solid #dBd8d1";
				oNode.style.backgroundColor = "#dBd8d1";
				break;
		}

		oNode.firstChild.style.margin = "1px";

		if (oNode.firstChild.filters) {
			oNode.firstChild.filters.item('Gray').enabled = (0 == oNode._state);
			oNode.firstChild.filters.item('Alpha').enabled = (0 == oNode._state);
			oNode.firstChild.filters.item("DropShadow").color = oNode.style.backgroundColor;
		}
	}

	function _callback_onmousedown() {
		if (0 == oNode._state)
			return;

		oNode.firstChild.style.margin = "1px";
		oNode.style.backgroundColor = "#a0a7bc";
		if (oNode.firstChild.filters)
			oNode.firstChild.filters.item("DropShadow").color = oNode.style.backgroundColor;
	}

	oNode.setDisabledState();

	oNode.attachEvent('onclick', _callback_onclick);
	oNode.attachEvent('onmouseover', _callback_onmouseover);
	oNode.attachEvent('onmouseout', _callback_onmouseout);
	oNode.attachEvent('onmousedown', _callback_onmousedown);
	oNode.attachEvent('onmouseup', _callback_onmouseover);

	return oNode;
}

SyndButton._nullInstance = null;
SyndButton.nullInstance = function() {
	if (null == SyndButton._nullInstance) {
		SyndButton._nullInstance = new Object();
		SyndButton._nullInstance.isNull = function() {return true;}
		SyndButton._nullInstance.setDisabledState = function() {}
		SyndButton._nullInstance.setDefaultState = function() {}
		SyndButton._nullInstance.setPressedState = function() {}
	}
	return SyndButton._nullInstance;
}
/////////////////////////////////////////////////