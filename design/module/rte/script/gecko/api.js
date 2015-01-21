/**
 * SyndRTE Gecko (Firefox/Mozilla/Netscape) api
 *
 * @access		public
 * @package		synd.rte.gecko
 */

/**
 * Override area callback initialization
 */
SyndEditor.lastSelectionAttach = function(oArea, fpFunction) {
	oArea.addEventListener('blur', fpFunction, true);
	oArea.addEventListener('keypress', _callback_gecko_onkeypress, true);
}

SyndArea._reverseAttachOrder = true;

function srte_api_selectNode(oNode) {
	if ('IMG' == oNode.tagName)
		oNode.click();
	else if (undefined != oNode.innerText) {
		var oRange = document.createRange();
		oRange.selectNode(oNode);
		oRange.select();
	}
}

function srte_api_selectNodeStart(oNode) {
	if (undefined != oNode.innerText)
		window.selection.collapse(oNode.firstChild, 0);
}

Event.prototype.__defineGetter__(
	'srcElement',
	function() { 
		return this.target; 
	}
);

/**
 * Cancel event bubbling
 * @param	boolean	Stop propagation
 */
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

// Exec Command /////////////////////////////////
function srte_api_execBold() {
	SyndRange.getSelection().toggleContainer('B');
}

function srte_api_execItalic() {
	SyndRange.getSelection().toggleContainer('EM');
}

function srte_api_execUnderline() {
	SyndRange.getSelection().toggleContainer('U');
}

function srte_api_execInsertUL () {
	window.getSelection().toggleListLines("UL", "OL");
}

function srte_api_execInsertOL () {
	window.getSelection().toggleListLines("OL", "UL");
}

/////////////////////////////////////////////////



// Node prototypes //////////////////////////////
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

Element.prototype.__defineGetter__(
	'currentStyle',
	function() {
		return window.getComputedStyle(this, null);
	}
);

Text.prototype.__defineGetter__(
	'currentStyle',
	function() {
		return this.parentNode.currentStyle;
	}
);

/**
 * Attach an event listener
 * @param	string		Event to attach to {onclick, onmouseover, ...}
 * @param	function	Function callback
 */
Element.prototype.attachEvent = function(sEvent, fpCallback) {
	this.addEventListener(sEvent.substring(2), fpCallback, false);
	if (undefined == this._events)
		this._events = new Object();
	if (undefined == this._events[sEvent])
		this._events[sEvent] = new Array();
	this._events[sEvent].push(fpCallback);
}

/**
 * Detach an event listener
 * @param	string		Event to attach to {onclick, onmouseover, ...}
 * @param	function	Function callback
 */
Element.prototype.detachEvent = function(sEvent, fpCallback) {
	this.removeEventListener(sEvent.substring(2), fpCallback, false);
	if (undefined == this._events || undefined == this._events[sEvent])
		return;
	for (var i=0; i<this._events[sEvent].length; i++) {
		if (this._events[sEvent][i] == fpCallback)
			this._events[sEvent].splice(i, 1);
	}
}

/**
 * Fire an event on this node
 * @param	string		Event to fire {onclick, onmouseover, ...}
 */
Element.prototype.fireEvent = function(sEvent) {
	var oEvent = new Object();
	oEvent.type = sEvent;
	oEvent.srcElement = this;
	this.bubbleEvent(oEvent);
}

/**
 * Bubbles an event up the tree
 * @access	protected
 * @param	object	Event to bubble
 */
Element.prototype.bubbleEvent = function(oEvent) {
	if (undefined != this._events && undefined != this._events[oEvent.type]) {
		for (var i=0; i<this._events[oEvent.type].length; i++)
			this._events[oEvent.type][i](oEvent);
	}
	if (null != this.parentNode && undefined != this.parentNode.bubbleEvent)
		this.parentNode.bubbleEvent(oEvent);
}

Element.prototype.__defineSetter__(
	'UNSELECTABLE',
	function(value) {
		if (value == "on") {
			this.style.MozUserSelect = "none";
			this.addEventListener('draggesture', function(oEvent) {
					oEvent.cancelBubble = true;
				}, false);
		}
		else 
			this.style.MozUserSelect = "text";
	}
);

Element.prototype.__defineSetter__(
	'contentEditable',
	function(bValue) {
		if(true == bValue) {
			this.style.MozUserModify = 'read-write';
			this.style.MozUserInput  = 'enabled';
      		this.style.MozUserFocus  = 'normal';
		}
		else {
			this.style.MozUserModify = 'read-only';
			this.style.MozUserInput  = 'disabled';
		}
	}
);

Element.prototype.__defineGetter__(
	'contentEditable',
	function() {
		return 'read-write' == this.style.MozUserModify;
	}
);

Window.prototype.attachEvent = Element.prototype.attachEvent;
Window.prototype.detachEvent = Element.prototype.detachEvent;

function srte_api_getSelRange() {
	return window.getSelection().getEditableRange();
}

// Emulate document.selection.createRange()
Window.prototype.__defineGetter__(
	'selection',
	function() {
		return this.getSelection();
	}
);

Selection.prototype.createRange = function() {
	return this.getEditableRange();
}

Document.prototype.__defineGetter__(
	'body',
	function() {
		return this.getElementsByTagName('BODY').item(0);
	}
);

CSSStyleDeclaration.prototype.removeAttribute = function(sAttribute) {
	this[sAttribute] = null;
	this.removeProperty(sAttribute);
}

/////////////////////////////////////////////////


// Range interface //////////////////////////////
function SyndRange() {}

SyndRange.getSelection = function() {
	return window.getSelection().getEditableRange();
}

SyndRange.createControlRange = function() {
	return document.createRange();
}

/**
 * Returns the innermost common ancestor
 * @return	object	Element
 */
Range.prototype.getParent = function() {
	return this.commonAncestorContainer;
}

Range.prototype.isEmpty = function() {
	return 0 == this.toString().length;
}

Range.prototype.add = function(oNode) {
}

Range.prototype.select = function() {
	if (!this.isEmpty()) {
		var oSelection = window.getSelection();
		oSelection.removeAllRanges();
		oSelection.addRange(this);
	}
}

/**
 * Replaces this range with a node, returns the new node
 * @param	object	Element	Node to replace range with
 * @return	object	Element
 */
Range.prototype.replaceNode = function(oNode) {
	this.deleteContents();
	this.firstInsertionPoint.insertNode(oNode);
	return oNode;
}

/**
 * Wraps this range inside a node
 * @return	object	Element
 */
Range.prototype.wrapInside = function(oNode) {
	var oTextNodes = this.textNodes;
	if (!oTextNodes.length)
		return;
	oTextNodes[0].parentNode.insertBefore(oNode, oTextNodes[0]);
	for (var i=0; i<oTextNodes.length; i++)
		oNode.appendChild(oTextNodes[i]);
	this.selectNodeContents(oNode);
	return oNode;
}

Range.prototype.createLink = function(sUri) {
	this.clearTextLinks();
	this.linkText(sUri);
}

Range.prototype.expandWord = function() {
	var iResult, oStartIp, oEndIp;

	oStartIp = this.firstInsertionPoint;
	oEndIp = this.lastInsertionPoint;

	while (' ' != oStartIp.character && !oStartIp.whitespace && InsertionPoint.SAME_LINE == (iResult = oStartIp.backOne()));
	if (InsertionPoint.CROSSED_BLOCK == iResult) 
		while (InsertionPoint.CROSSED_BLOCK != oStartIp.forwardOne());
	else
		oStartIp.forwardOne();

	while (' ' != oEndIp.character && !oEndIp.whitespace && InsertionPoint.SAME_LINE == (iResult = oEndIp.forwardOne()));
	if (InsertionPoint.CROSSED_BLOCK == iResult) 
		while (InsertionPoint.CROSSED_BLOCK != oEndIp.backOne());
	else
		oStartIp.backOne();

	this.setStart(oStartIp.ipNode, oStartIp.ipOffset);
	this.setEnd(oEndIp.ipNode, oEndIp.ipOffset);
}

Range.prototype.expandBlock = function() {
	var oLines = this.lines;
	var oFirst = oLines[0].firstInsertionPoint;
	var oLast  = oLines[oLines.length-1].lastInsertionPoint;
	this.setStart(oFirst.ipNode, oFirst.ipOffset);
	this.setEnd(oLast.ipNode, oLast.ipOffset);
}

Range.prototype.toggleContainer = function(sTag) {
	if (null != (oParent = SyndLib.findParent(this.getParent(), sTag))) {
		if (this.isEmpty()) {
			this.expandWord();
		}
		else {
			var oTextNodes = this.textNodes;
			this.__markTextBoundaries();

			// Split parent at first range boundary?
			if (oTextNodes[0].parentNode.childNodes.length > 1)
				oParent = oTextNodes[0].parentNode.split(oTextNodes[0].offset);

			// Split parent at last range boundary?
			if (oTextNodes[oTextNodes.length-1].parentNode.childNodes.length > 1)
				oTextNodes[oTextNodes.length-1].parentNode.split(oTextNodes[oTextNodes.length-1].offset+1);

			srte_core_inlineNode(oParent);
	
			this.__restoreTextBoundaries();
			this.select();
		}
	}
	else {
		if (this.isEmpty()) {
			// Store cursor position and expand word
			var iOffset = this.firstInsertionPoint.ipOffset;
			this.expandWord();
			iOffset -= this.firstInsertionPoint.ipOffset;
			
			// Wrap range and restore cursor offset
			var oNode = this.wrapInside(document.createElement(sTag));
			this.selectNode(oNode);
			this.normalizeElements(sTag);
			
			window.getSelection().collapse(oNode.firstChild, iOffset);
		}
		else {
			this.__markTextBoundaries();

			var oNode = this.wrapInside(document.createElement(sTag));
			if (null != SyndLib.findParent(this.getParent().parentNode, sTag))
				srte_core_inlineNode(oNode);
			
			var oRange = document.createRange();
			oRange.selectNode(this.getParent().parentNode);
			oRange.normalizeElements(sTag);
			oRange.detach();

			this.__restoreTextBoundaries();
			this.select();
		}
	}
}
/////////////////////////////////////////////////


/**
 * Gecko keyboard handler
 * @access	private
 */
function _callback_gecko_onkeypress(oEvent) {
	if(oEvent.ctrlKey || oEvent.metaKey) {
		switch (String.fromCharCode(oEvent.charCode).toLowerCase()) {
			case 'v':
				oEvent.cancelBubble = window.getSelection().paste();
				SyndArea._changed = true;
				break;
			case 'x':
				oEvent.cancelBubble = window.getSelection().cut();
				SyndArea._changed = true;
				break;
			case 'c':
				oEvent.cancelBubble = window.getSelection().copy();
				break;
			case 'b':
				oEvent.cancelBubble = true;
				srte_api_execBold();
				SyndArea._changed = true;
				break;
			case 'i':
				oEvent.cancelBubble = true;
				srte_api_execItalic();
				SyndArea._changed = true;
				break;
			case 'u':
				oEvent.cancelBubble = true;
				srte_api_execUnderline();
				SyndArea._changed = true;
				break;
			case 's':
				oEvent.cancelBubble = true;
				SyndEditor.saveDocument();
				break;
		}
	} 
	else {
		if (oEvent.DOM_VK_BACK_SPACE == oEvent.keyCode) {
			oEvent.cancelBubble = window.getSelection().deleteSelection()
			SyndArea._changed = true;
		}
		else if (oEvent.DOM_VK_DELETE == oEvent.keyCode) {
			var oSelection = window.getSelection();
			if (oSelection.isCollapsed)
				oSelection.moveInsertionPoint(Selection.MOVE_FORWARD_ONE);
			oEvent.cancelBubble = oSelection.deleteSelection()
			SyndArea._changed = true;
		}
		else if (oEvent.DOM_VK_LEFT == oEvent.keyCode && !oEvent.shiftKey)
			oEvent.cancelBubble = window.getSelection().moveInsertionPoint(Selection.MOVE_BACK_ONE);
		else if (oEvent.DOM_VK_RIGHT == oEvent.keyCode && !oEvent.shiftKey)
			oEvent.cancelBubble = window.getSelection().moveInsertionPoint(Selection.MOVE_FORWARD_ONE);
		else if (oEvent.DOM_VK_UP == oEvent.keyCode && !oEvent.shiftKey)
			oEvent.cancelBubble = window.getSelection().moveInsertionPoint(Selection.MOVE_UP_ONE);
		else if (oEvent.DOM_VK_DOWN == oEvent.keyCode && !oEvent.shiftKey)
			oEvent.cancelBubble = window.getSelection().moveInsertionPoint(Selection.MOVE_DOWN_ONE);
		else if (oEvent.DOM_VK_RETURN == oEvent.keyCode) {
			oEvent.cancelBubble = window.getSelection().splitXHTMLLine(oEvent.shiftKey);
			SyndArea._changed = true;
		}
		// Home goes to start of line/block
		else if (oEvent.DOM_VK_HOME == oEvent.keyCode) {
			var oRange = window.getSelection().getEditableRange();
			if (null != oRange) {
				var oPoint = oRange.lines[0].firstInsertionPoint;
				window.getSelection().collapse(oPoint.ipNode, oPoint.ipOffset);
				oEvent.cancelBubble = true;
			}
		}
		// End goes to end of line/block
		else if (oEvent.DOM_VK_END == oEvent.keyCode) {
			var oRange = window.getSelection().getEditableRange();
			if (null != oRange) {
				var oPoint = oRange.lines[0].lastInsertionPoint; 
				window.getSelection().collapse(oPoint.ipNode, oPoint.ipOffset);
				oEvent.cancelBubble = true;
			}
		}
		// Alphanumeric
		else if (oEvent.charCode) {
			oEvent.cancelBubble = window.getSelection().insertCharacter(oEvent.charCode);
			SyndArea._changed = true;
		}
	}
}
