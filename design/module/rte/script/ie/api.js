/**
 * SyndRTE Internet Explorer (IE6) api
 *
 * @access		public
 * @package		synd.rte.ie
 */

/**
 * Override area callback initialization
 */
SyndEditor.lastSelectionAttach = function(oArea, fpFunction) {
	oArea.attachEvent('onbeforedeactivate', fpFunction);
	oArea.attachEvent('onkeyup', _callback_ie_onkeyup);
}

function _callback_ie_onkeyup(oEvent) {
	if (oEvent.ctrlKey || oEvent.metaKey) {
		switch (String.fromCharCode(oEvent.keyCode).toLowerCase()) {
			case 's':
				oEvent.cancelBubble = true;
				SyndEditor.saveDocument();
				break;
		}
	}
}



// Range interface //////////////////////////////
function SyndRange() {}

SyndRange.getSelection = function() {
	if (null == (oRange = document.selection.createRange()))
		return null;
	if (undefined == oRange.text)
		return new SyndControlRange(oRange);
	return new SyndTextRange(oRange);
}

SyndRange.createControlRange = function() {
	return new SyndControlRange();
}

/** /////////////////////////////////////////////
 * A character range
 */
function SyndTextRange(oRange) {
	if (undefined == oRange)
		this._range = document.body.createTextRange();
	else
		this._range = oRange;
}

/**
 * Returns the innermost common ancestor
 * @return	object	Element
 */
SyndTextRange.prototype.getParent = function() {
	return this._range.parentElement();
}

SyndTextRange.prototype.toString = function() {
	return this._range.text;
}

SyndTextRange.prototype.isEmpty = function() {
	return 0 == this._range.text.length;
}

SyndTextRange.prototype.isContained = function(oNode) {
	var oRange = document.body.createTextRange();
	oRange.moveToElementText(oNode);
	return this._range.inRange(oRange) ||
		this._range.compareEndPoints('StartToStart', oRange) <= 0 && this._range.compareEndPoints('EndToEnd', oRange) >= 0 ||
		this._range.compareEndPoints('StartToStart', oRange) >= 0 && this._range.compareEndPoints('StartToEnd', oRange) <= 0 ||
		this._range.compareEndPoints('EndToEnd', oRange) <= 0 && this._range.compareEndPoints('EndToStart', oRange) >= 0;
}

SyndTextRange.prototype.expandWord = function() {
	this._range.moveStart('word', -1);
	this._range.moveEnd('word');
}

SyndTextRange.prototype.expandBlock = function() {
	var c, bRet = true, iLength = this._range.text.length-1;

	while (bRet && Math.abs(bRet) == this._range.text.length-iLength) {
		c = this._range.text.charCodeAt(0);
		if ((c >= 97 && c <= 122) || (c >= 65 && c <= 90)) 
			this._range.moveStart('word', -1);

		iLength = this._range.text.length;
		bRet = this._range.moveStart('character', -1);
	}
	this._range.moveStart('character', 1);

	bRet 	= true;
	iLength = this._range.text.length-1;
	
	while (bRet && Math.abs(bRet) == this._range.text.length-iLength) {
		c = this._range.text.charCodeAt(this._range.text.length-1);
		if ((c >= 97 && c <= 122) || (c >= 65 && c <= 90)) 
			this._range.moveEnd('word');

		iLength = this._range.text.length;
		bRet = this._range.moveEnd('character');
	}

	bRet 	= true;
	iLength = this._range.text.length-1;

	while (bRet && Math.abs(bRet) != this._range.text.length-iLength) {
		iLength = this._range.text.length;
		bRet = this._range.moveEnd('character');
	}
	
	this._range.moveEnd('character', -1);
}

SyndTextRange.prototype.removeNodes = function(sTagName) {
	var oNodes = this.getParent().getElementsByTagName(sTagName);
	for (var i=0; i<oNodes.length; i++) {
		if (this.isContained(oNodes.item(i)))
			srte_core_inlineNode(oNodes.item(i));
	}
}

/**
 * Replaces this range with a node, returns the new node
 * @param	object	Element	Node to replace range with
 * @return	object	Element
 */
SyndTextRange.prototype.replaceNode = function(oNode) {
	var sOldId = oNode.id;
	oNode.id = '_srte_node';

	this._range.pasteHTML(oNode.outerHTML);

	oNode = document.getElementById('_srte_node');
	oNode.id = sOldId;
	return oNode;
}

/**
 * Wraps this range inside a node
 * @return	object	Element
 */
SyndTextRange.prototype.wrapInside = function(oNode) {
	oNode.innerHTML = this._range.htmlText; 
	return this.replaceNode(oNode);
}

/**
 * Adjust the start and end not to include any whitespace
 */
SyndTextRange.prototype.collapseWhitespace = function() {
	var sTrim = " \n\t\r";
	while (this._range.text.length && -1 != sTrim.indexOf(this._range.text.charAt(0)))
		this._range.moveStart('character');
	while (this._range.text.length && -1 != sTrim.indexOf(this._range.text.charAt(this._range.text.length-1)))
		this._range.moveEnd('character', -1);
}

/**
 * Creates a new link
 * @param	string	URI to link to
 * @return	object	Element
 */
SyndTextRange.prototype.createLink = function(sUri) {
	var oLink = srte_core_getParentByTag(this.getParent(), 'A');
	if (null != oLink)
		srte_core_inlineNode(oLink);
	this.collapseWhitespace();
	this.removeNodes('A');
	oLink = this.wrapInside(document.createElement('A'));
	oLink.href = sUri;
}

SyndTextRange.prototype.select = function() {
	this._range.select();
}

/** /////////////////////////////////////////////
 * A set of nodes
 */
function SyndControlRange(oRange) {
	if (undefined == oRange)
		this._range = document.body.createControlRange();
	else
		this._range = oRange;
}

/**
 * Returns the innermost node container
 * @return	object	Element
 */
SyndControlRange.prototype.getParent = function() {
	return this._range.item(0);
}

SyndControlRange.prototype.toString = function() {
	var sText = '';
	for (i=0; i<this._range.length; i++)
		sText += this._range.item(i).innerText;
	return sText;
}

SyndControlRange.prototype.isEmpty = function() {
	return 0 == this._range.length;
}

SyndControlRange.prototype.expandWord = function() {}
SyndControlRange.prototype.expandBlock = function() {}

SyndControlRange.prototype.removeNodes = function(sTagName) {
	for (i=0; i<this._range.length; i++) {
		if (this._range.item(i).tagName == sTagName)
			this._range.item(i).parentNode.removeChild(this._range.item(i));
	}
}

/**
 * Wraps this range inside a node
 * @return	object	Element
 */
SyndControlRange.prototype.wrapInside = function(oNode) {
	this._range.item(0).parentNode.insertBefore(oNode, this._range.item(0));
	for (i=0; i<this._range.length; i++)
		oNode.appendChild(this._range.item(i));
	return oNode;
}

/**
 * Replaces this range with a node, returns the new node
 * @param	object	Element	Node to replace range with
 * @return	object	Element
 */
SyndControlRange.prototype.replaceNode = function(oNode) {
	var oParent = this._range.item(0).parentNode;
	oParent.insertBefore(oNode, this._range.item(0));
	for (var i=0; i<this._range.length; i++)
		this._range.item(i).parentNode.removeChild(this._range.item(i));
	return oNode;
}

/**
 * Creates a new link
 * @param	string	URI to link to
 * @return	object	Element
 */
SyndControlRange.prototype.createLink = function(sUri) {
	var oLink = srte_core_getParentByTag(this.getParent(), 'A');
	if (null != oLink)
		srte_core_inlineNode(oLink);
	this.removeNodes('A');
	oLink = this.wrapInside(document.createElement('A'));
	oLink.href = sUri;
}

SyndControlRange.prototype.add = function(oNode) {
	this._range.add(oNode);
}

SyndControlRange.prototype.select = function() {
	this._range.select();
}
/////////////////////////////////////////////////






function srte_api_selectNode(oNode) {
	if ('IMG' == oNode.tagName) {
		oNode.click();
	}
	else if (undefined != oNode.innerText) {
		var oRange = document.body.createTextRange();
		oRange.moveToElementText(oNode);
		oRange.select();
	}
}

function srte_api_selectNodeStart(oNode) {
	if (undefined != oNode.innerText) {
		var oRange = document.body.createTextRange();
		oRange.moveToElementText(oNode);
		oRange.collapse();
		oRange.select();
	}
}


// Exec Command /////////////////////////////////
function srte_api_execBold() {
	document.execCommand('Bold');
}

function srte_api_execItalic() {
	document.execCommand('Italic');
}

function srte_api_execUnderline() {
	document.execCommand('Underline');
}

function srte_api_execInsertUL () {
	document.execCommand('InsertUnorderedList');
}

function srte_api_execInsertOL () {
	document.execCommand('InsertOrderedList');
}
/////////////////////////////////////////////////
