/**
 * SyndRTE core and library functions.
 *
 * @access		public
 * @package		synd.rte
 */

/** /////////////////////////////////////////////
 * Main editor singleton
 */
function SyndEditor(oDocument) {
	var oThis = this;
	
	SyndEditor.scanDocument(oDocument, 'DIV');
	SyndEditor.scanDocument(oDocument, 'TEXTAREA');

	var oForms = oDocument.getElementsByTagName('FORM');
	for (var i=0; i<oForms.length; i++)
		oForms.item(i).attachEvent('onsubmit', SyndEditor._callback_onsubmit);
	
	// Initialize toolbars
	new SyndButtonToolbar(oDocument.getElementById('srte_toolbar_button'));
	new SyndPropertiesToolbar(oDocument.getElementById('srte_toolbar_properties'));
	new SyndDOMToolbar(oDocument.getElementById('srte_toolbar_dom'));

	SyndEditor.fireEvent('onload');
	SyndEditor.fireEvent('onactivate', null);
}

SyndEditor._lastSelection = null;
SyndEditor._lastLength = -1;
SyndEditor._lastNode = null;
SyndEditor._events = new Object();

/**
 * Scans for and initalizes areas
 */
SyndEditor.scanDocument = function(oDocument, sTag) {
	var oAll = oDocument.getElementsByTagName(sTag);
	for (var i=0; i<oAll.length; i++) {
		if ('srte_area' != oAll.item(i).className)
			continue;
		new SyndArea(oAll.item(i));
	}
}

SyndEditor.saveDocument = function() {
	var oForm, oArea = SyndArea.getActive();

	if (null != oArea && null != (oForm = srte_core_getParentByTag(oArea.getNode(), 'FORM', true))) {
		oInput = document.createElement('INPUT');
		oInput.type  = 'hidden';
		oInput.name  = 'post';
		oInput.value = '1';
		oForm.appendChild(oInput);
		oForm.fireEvent('onsubmit');
		oForm.submit();
	}
}

SyndEditor.attachArea = function(oNode) {
	// Hook up save-last-selection
	SyndEditor.lastSelectionAttach(oNode, function() {
			SyndEditor._lastSelection = SyndRange.getSelection();
		});

	// Hook up cursor movement events
	oNode.attachEvent('oncontrolselect', SyndEditor._callback_oncursor);
	oNode.attachEvent('onclick', SyndEditor._callback_oncursor);
	oNode.attachEvent('ondblclick', SyndEditor._callback_oncursor);
	oNode.attachEvent('onkeyup', SyndEditor._callback_oncursor);
	oNode.attachEvent('onselectstart', SyndEditor._callback_oncursor);

	// Hook up context menu
	oNode.attachEvent('oncontextmenu', SyndEditor._callback_oncontextmenu);
}

SyndEditor.lastSelectionAttach = function(oArea, fpFunction) {
	oArea.attachEvent('onbeforedeactivate', fpFunction);
}

SyndEditor.getLastSelection = function() {
	return SyndEditor._lastSelection;
}

SyndEditor.getActiveNode = function() {
	if (null != SyndEditor._lastNode)
		return SyndEditor._lastNode;
	return SyndRange.getSelection().getParent();
}

SyndEditor.activateNode = function(oNode) {
	SyndEditor._lastNode = oNode;
	SyndEditor.fireEvent('onactivate', oNode);
}

SyndEditor.attachEvent = function(sEvent, fpCallback) {
	if (undefined == SyndEditor._events[sEvent])
		SyndEditor._events[sEvent] = new Array();
	SyndEditor._events[sEvent].push(fpCallback);
}

SyndEditor.detachEvent = function(sEvent, fpCallback) {
	if (undefined == SyndEditor._events[sEvent])
		return;
	for (var i=0; i<SyndEditor._events[sEvent].length; i++) {
		if (SyndEditor._events[sEvent][i] == fpCallback)
			SyndEditor._events[sEvent].splice(i, 1);
	}
}

SyndEditor.fireEvent = function(sEvent) {
	if (undefined == SyndEditor._events[sEvent])
		return;
	for (var i=0; i<SyndEditor._events[sEvent].length; i++)
		SyndEditor._events[sEvent][i](arguments[1], arguments[2]);
}

SyndEditor._callback_oncursor = function(oEvent) {
	var oNode = oEvent.srcElement;
	if ('keyup' == oEvent.type || 'srte_area' == oNode.className)
		oNode = SyndRange.getSelection().getParent();
	if (null == oNode)
		return;

	if (oNode != SyndEditor._lastNode)
		SyndEditor.activateNode(oNode);
	else {
		var iLength = SyndRange.getSelection().toString().length;
		if (SyndEditor._lastLength != iLength) {
			SyndEditor._lastLength = iLength;
			SyndEditor.activateNode(oNode);
		}
	}
}

SyndEditor._callback_onsubmit = function(oEvent) {
	var oInput, oAreas = SyndArea.getAll();
	SyndArea._changed = false;
	
	for (var i=0; i<oAreas.length; i++) {
		oInput = document.createElement('input');
		oInput.type = 'hidden';
		oInput.name = oInput.id = oAreas[i].getName();
		oInput.value = oAreas[i].getHtml();
		oEvent.srcElement.appendChild(oInput);
	}
}

SyndEditor._callback_oncontextmenu = function(oEvent) {
	if ('TABLE' == oEvent.srcElement.tagName)
		return SyndEditor.contextMenu(oEvent, oEvent.srcElement);
}

SyndEditor.contextMenu = function(oEvent, oNode) {
	document.body.fireEvent('onclick');

	var oMenu = document.createElement('UL');
	oMenu.className = 'srte_context_menu';
	oMenu.style.left = oEvent.clientX+'px';
	oMenu.style.top = oEvent.clientY+'px';
	
	oMenu.appendChild(SyndEditor.contextMenuItem('Select Tag', 
		function() {SyndEditor.selectNode(oNode)}));
	oMenu.appendChild(SyndEditor.contextMenuItem('Remove Tag', function() {
		SyndEditor.activateNode(srte_core_inlineNode(oNode))}));
	oMenu.appendChild(SyndEditor.contextMenuItem('Delete', function() {
			var oParent = oNode.parentNode; 
			oNode.parentNode.removeChild(oNode);
			SyndEditor.activateNode(oParent);
		}));
		
	SyndEditor.fireEvent('oncontextmenu', oNode, oMenu);

	function _callback_close() {
		document.body.detachEvent('onclick', _callback_close);
		document.body.detachEvent('oncontextmenu', _callback_close);
		document.body.detachEvent('onkeydown', _callback_close);
		document.body.removeChild(oMenu);
	}

	document.body.appendChild(oMenu);
	document.body.attachEvent('onclick', _callback_close);
	document.body.attachEvent('oncontextmenu', _callback_close);
	document.body.attachEvent('onkeydown', _callback_close);

	oEvent.cancelBubble = true;
	return false;
}

SyndEditor.contextMenuItem = function(sTitle, fpCallback) {
	var oNode = document.createElement('LI');
	oNode.innerText = sTitle;
	oNode.attachEvent('onmouseover', function() 
		{oNode.className='srte_context_hover'});
	oNode.attachEvent('onmouseout', function() 
		{oNode.className=null});
	oNode.attachEvent('onclick', fpCallback);
	return oNode;
}

SyndEditor.selectNode = function(oNode) {
	srte_api_selectNode(oNode);
	SyndEditor.activateNode(oNode);
}

SyndEditor.controlNode = function(oNode) {
	var oRange = SyndRange.createControlRange();
	oRange.add(oNode);
	oRange.select();
}
/////////////////////////////////////////////////


/** Table behavior //////////////////////////////
 *
 */
function SyndTable(oTable) {
	this._table = oTable;
	this._checkStructure();
}

SyndTable.prototype._table = null;

/**
 * Table factory method
 * @return	object	SyndTable
 */
SyndTable.factory = function(oNode) {
	if ('TBODY' == oNode.tagName)
		return new SyndTable(oNode.parentNode);
	return new SyndTable(oNode);
}

/**
 * Checks for a TBODY and creates one if not found
 * @access	private
 */
SyndTable.prototype._checkStructure = function() {
	if ('TR' == this._table.firstChild.tagName) {
		var oBody = this._table.appendChild(document.createElement('TBODY'));
		while (this._table.childNodes.length > 1)
			oBody.appendChild(this._table.removeChild(this._table.childNodes.item(0)));
	}
}

/**
 * Table context menu callback
 * @access	private
 */
SyndTable._callback_oncontextmenu = function(oNode, oMenu) {
	if ('TABLE' == oNode.tagName || 'TBODY' == oNode.tagName) {
		oMenu.appendChild(document.createElement('HR'));
		oMenu.appendChild(SyndEditor.contextMenuItem('Insert Row', function() {
				var oTable = SyndTable.factory(oNode);
				oTable.insertRow();
				SyndEditor.controlNode(oNode);
				SyndEditor.activateNode(oNode);
			}));
		oMenu.appendChild(SyndEditor.contextMenuItem('Insert Column', function() {
				var oTable = SyndTable.factory(oNode);
				oTable.insertColumn();
				SyndEditor.controlNode(oNode);
				SyndEditor.activateNode(oNode);
			}));
	}
	else if ('TD' == oNode.tagName || 'TH' == oNode.tagName) {
		oMenu.appendChild(document.createElement('HR'));
		oMenu.appendChild(SyndEditor.contextMenuItem('Increase Row Span', function() {
				oNode.rowSpan++;
			}));
		oMenu.appendChild(SyndEditor.contextMenuItem('Increase Column Span', function() {
				oNode.colSpan++;
			}));
		oMenu.appendChild(SyndEditor.contextMenuItem('Decrease Row Span', function() {
				if (oNode.rowSpan > 1)
					oNode.rowSpan--;
			}));
		oMenu.appendChild(SyndEditor.contextMenuItem('Decrease Column Span', function() {
				if (oNode.colSpan > 1)
					oNode.colSpan--;
			}));
	}
}

// Attach oncontextmenu callback
SyndEditor.attachEvent('oncontextmenu', SyndTable._callback_oncontextmenu);

/**
 * Table cell <td> factory method
 * @return	object	Element
 */
SyndTable.createCell = function() {
	var oCell = document.createElement('TD');
	oCell.innerHTML = '&nbsp;';
	return oCell;
}

SyndTable.prototype.getRows = function() {
	var oRows = new Array(), oItem;
	for (var i=0; i<this._table.childNodes.length; i++) {
		// Descend THEAD/TBODY/TFOOT
		oItem = this._table.childNodes.item(i);
		for (var j=0; j<oItem.childNodes.length; j++) {
			if ('TR' == oItem.childNodes.item(j).tagName) 
				oRows.push(oItem.childNodes.item(j));
		}
	}
	return oRows;
}

SyndTable.prototype.getLastRow = function() {
	var oItem;
	for (var i=this._table.childNodes.length-1; i>=0; i--) {
		// Descend THEAD/TBODY/TFOOT
		oItem = this._table.childNodes.item(i);
		for (var j=oItem.childNodes.length-1; j>=0; j--) {
			if ('TR' == oItem.childNodes.item(j).tagName) 
				return oItem.childNodes.item(j);
		}
	}
	return null;
}

SyndTable.prototype.insertRow = function() {
	if (null != (oRow = this._table.insertRow(this.getRowCount()))) {
		if (0 != (iCount = this.getColumnCount())) {
			for (var i=0; i<iCount; i++)
				oRow.appendChild(SyndTable.createCell());
		}
		else {
			oRow.appendChild(SyndTable.createCell());
			oRow.appendChild(SyndTable.createCell());
		}
	}
}

SyndTable.prototype.insertColumn = function() {
	var oRows = this.getRows();
	for (var i=0; i<oRows.length; i++)
		oRows[i].appendChild(SyndTable.createCell());
}

SyndTable.prototype.getRowCount = function() {
	return this.getRows().length;
}

SyndTable.prototype.setRowCount = function(iValue) {
	if (iValue < 1) return;
	var iRows = this.getRowCount();

	// Remove trailing rows
	if (iRows > iValue) {
		var oRow = null;
		for (; iRows > iValue; iRows--) {
			oRow = this.getLastRow();
			oRow.parentNode.removeChild(oRow);
		}
	}
	// Append new rows
	else if (iRows < iValue) {
		for (; iRows < iValue; iRows++)
			this.insertRow();
	}
}

SyndTable.prototype.getColumnCount = function() {
	var iColumns = 0, iCount = 0, oItem;
	for (var i=0; i<this._table.childNodes.length; i++) {
		// Descend THEAD/TBODY/TFOOT
		oItem = this._table.childNodes.item(i);
		for (var j=0; j<oItem.childNodes.length; j++) {
			if ('TR' == oItem.childNodes.item(j).tagName) {
				if (iColumns < (iCount = this._countNodes(oItem.childNodes.item(j).childNodes, 'TD')))
					iColumns = iCount;
			}
		}
	}
	return iColumns;
}

SyndTable.prototype._countNodes = function(oCollection, sTag) {
	var iCount = 0;

	for (var i=0; i<oCollection.length; i++) {
		if (sTag == oCollection.item(i).tagName)
			iCount++;
	}
	return iCount;
}

SyndTable.prototype._findLast = function(oCollection, sTag) {
	for (var i=oCollection.length-1; i>=0; i--) {
		if (sTag == oCollection.item(i).tagName)
			return oCollection.item(i);
	}
	return null;
}

SyndTable.prototype.setColumnCount = function(iValue) {
	if (iValue < 1) return;
	var iColumns = this.getColumnCount(), oRows = this.getRows(), i;

	// Remove trailing cells
	if (iColumns > iValue) {
		for (i=0; i<oRows.length; i++) {
			while (this._countNodes(oRows[i].childNodes, 'TD') > iValue) {
				if (null != (oChild = this._findLast(oRows[i].childNodes, 'TD')))
					oRows[i].removeChild(oChild);
			}
		}
	}
	// Append new cells
	else if (iColumns < iValue) {
		for (i=0; i<oRows.length; i++) {
			while (this._countNodes(oRows[i].childNodes, 'TD') < iValue)
				oRows[i].appendChild(SyndTable.createCell());
		}
	}
}

SyndTable.prototype.getNode = function() {
	return this._table;
}

SyndTable.prototype.getWidth = function() {
	return this._table.style.width;
}

SyndTable.prototype.setWidth = function(sValue) {
	this._table.style.width = sValue;
}

SyndTable.prototype.getHeight = function() {
	return this._table.style.height;
}

SyndTable.prototype.setHeight = function(sValue) {
	this._table.style.height = sValue;
}

SyndTable.prototype.getCellPadding = function() {
	return this._table.cellPadding;
}

SyndTable.prototype.setCellPadding = function(sValue) {
	this._table.cellPadding = sValue;
}

SyndTable.prototype.getCellSpacing = function() {
	return this._table.cellSpacing;
}

SyndTable.prototype.setCellSpacing = function(sValue) {
	this._table.cellSpacing = sValue;
}

SyndTable.prototype.getBorderWidth = function() {
	return this._table.border;
}

SyndTable.prototype.setBorderWidth = function(sValue) {
	this._table.border = sValue;
}
/////////////////////////////////////////////////



/** /////////////////////////////////////////////
 * Static utility functions
 */
function SyndLib() {}

SyndLib.trim = function(sValue) {
	if (undefined != sValue && null != sValue) {

		var sTrim = " \n\t\r";
		var i=0, j=sValue.length-1;
		while (i < sValue.length && -1 != sTrim.indexOf(sValue.charAt(i)))
			i++;
		while (j >= i && -1 != sTrim.indexOf(sValue.charAt(j)))
			j--;
		return sValue.slice(i, j+1);
	}
	return '';
}

SyndLib.findParent = function(oNode, sTag) {
	while (oNode && 'srte_area' != oNode.className) {
		if (oNode.tagName == sTag)
			return oNode;
		oNode = oNode.parentNode;
	}
	return null;
}

SyndLib.findParentByTags = function(oNode, oTags) {
	while (oNode && 'srte_area' != oNode.className) {
		if (true == oTags[oNode.tagName])
			return oNode;
		oNode = oNode.parentNode;
	}
	return null;
}

SyndLib.syntaxHighlight = function(sCode) {
	var re, sBrTags = 'div|p';

	// Newline after <br> that's not followed  by <div> or <p>
	sCode = sCode.replace(/(&lt;br\s?\/?&gt;)(?!\s*&lt;(div|p)&gt;)/igm, '$1<br />');

	// Indent blocktags
	re    = new RegExp('(&lt;('+sBrTags+')[^&]*&gt;)', 'igm');
	sCode = sCode.replace(re, '<br />$1<div style="margin-left:20px">');

	// Newline after blocktags
	re    = new RegExp('(&lt;\/('+sBrTags+')&gt;)', 'igm');
	sCode = sCode.replace(re, '</div>$1<br />');

	// Highlight tags
	re    = new RegExp('((?:&lt;)\/?[a-zA-Z]{1,10}(?:&gt;|\\s+))', 'igm');
	sCode = sCode.replace(re,
		function($0) {
			return '<span style="color:blue;">'+
					$0.toLowerCase()+
				   '</span>';
		});

	// Highlight < > <?= ?>
	sCode = sCode.replace(/(&lt;\??=?|\??&gt;)/igm, '<span style="color:blue;">$1</span>');

	// Set font and size
	sCode = '<span style="font-size:10px; font-family:courier">'+sCode+'</span>';

	return sCode;
}

function srte_core_getParentByTag(oNode, sTag, bFullTree) {
	while (oNode && ('srte_area' != oNode.className || bFullTree)) {
		if (oNode.tagName == sTag)
			return oNode;
		oNode = oNode.parentNode;
	}
	return null;
}

function srte_core_getParentByStyle(oNode, sKey) {
	while (oNode && 'srte_area' != oNode.className) {
		if ('' != oNode.style.getAttribute(sKey))
			return oNode;
		oNode = oNode.parentNode;
	}
	return null;
}

function srte_core_getParentByClass(oNode, sClass, bFullTree) {
	while (oNode && ('srte_area' != oNode.className || bFullTree)) {
		if (sClass == oNode.className)
			return oNode;
		oNode = oNode.parentNode;
	}
	return null;
}

/**
 * Wrap the current selection inside a node
 * @param	object
 * @return	mixed
 */
function _srte_core_wrapSel(oContainer) {
	var oNode = SyndEditor.getActiveNode();
	var oRange = SyndRange.getSelection();
	
	// If the current node is fully selected
	if (SyndLib.trim(oNode.innerText).length == SyndLib.trim(oRange.toString()).length) {
		oContainer.appendChild(oNode.replaceNode(oContainer));
		return oContainer;
	}

	// Wrap selected range
	if (!oRange.isEmpty())
		return oRange.wrapInside(oContainer);
	return null;
}

function srte_core_getParentByTagsEx(oNode, oTags, iLength) {
	var oCandidate = null;
	while (oNode && 'srte_area' != oNode.className && SyndLib.trim(oNode.innerText).length <= iLength) {
		if (true == oTags[oNode.tagName])
			oCandidate = oNode;
		oNode = oNode.parentNode;
	}
	return oCandidate;
}

function srte_core_normalizeNode(oNode) {
	var oChild, oNodes, i;
	for (i=0; i<oNode.childNodes.length; i++) {
		oChild = oNode.childNodes.item(i);
		if ('SPAN' == oChild.tagName && 0 == SyndLib.trim(oChild.innerText)) 
			oChild.outerText = oChild.innerText;
	}

	for (i=0; i<oNode.childNodes.length; i++) {
		oChild = oNode.childNodes.item(i);
		oNodes = oChild.childNodes;
		if ('DIV' == oChild.tagName && oNode.className == oChild.className && '' == oChild.id) {
			if (0 == oNodes.length || 
				'BR' != oNodes.item(oNodes.length-1).tagName || 
				('BR' == oNodes.item(oNodes.length-1).tagName && 
					(oNodes.length < 2 || 'BR' != oNodes.item(oNodes.length-2).tagName)))
			{
				oChild.insertAdjacentElement('afterEnd', document.createElement('BR'));
			}
			srte_core_inlineNode(oChild);
			i--;
		}
	}
}

function srte_core_insertAfter(oNode, oChild) {
	var oParent = oChild.parentNode;
	if(oParent.lastChild == oChild)
		oParent.appendChild(oNode);
	else
		oParent.insertBefore(oNode, oChild.nextSibling);
}

function srte_core_inlineNode(oNode) {
	var oParent = oNode.parentNode;
	while (oNode.childNodes.length) 
		oParent.insertBefore(oNode.removeChild(oNode.childNodes.item(0)), oNode);
	oParent.removeChild(oNode);
	return oParent;
}

function srte_core_mergeNode(oNode, oMergeNode) {
	while (oNode.childNodes.length) 
		oMergeNode.appendChild(oNode.removeChild(oNode.childNodes.item(0)));
		
	for (var i=0; i<oNode.attributes.length; i++) {
		if (oNode.attributes[i].specified)
			oMergeNode.setAttribute(oNode.attributes[i].name, oNode.attributes[i].value);
	}

	if (oNode.attributes['style'].specified)

		oMergeNode.style.cssText = oNode.style.cssText;
	
	oNode.parentNode.replaceChild(oMergeNode, oNode);

	if (oNode.id)
		oMergeNode.id = oNode.id;
	if (oNode.className)
		oMergeNode.className = oNode.className;

	srte_api_selectNodeStart(oMergeNode);

	return oMergeNode;
}




function srte_core_getSelBlockCreate(oTags, sDefaultTag) {
	var oNode, oRange = SyndRange.getSelection();
	oRange.expandBlock();

	oNode = srte_core_getParentByTagsEx(oRange.getParent(), oTags, SyndLib.trim(oRange.toString()).length);
	if (null != oNode)
		return oNode;
	
	if (!oRange.isEmpty()) {
		oNode = oRange.wrapInside(document.createElement(sDefaultTag));

		while (oNode.firstChild && 'BR' == oNode.firstChild.tagName)
			oNode.parentNode.insertBefore(oNode.removeChild(oNode.firstChild), oNode);
		while (oNode.lastChild && 'BR' == oNode.lastChild.tagName)
			srte_core_insertAfter(oNode.removeChild(oNode.lastChild), oNode);

		if (oNode.nextSibling && 'BR' == oNode.nextSibling.tagName)
			oNode.parentNode.removeChild(oNode.nextSibling);
		
		srte_api_selectNodeStart(oNode);
		return oNode;
	}
	
	return null;
}


function srte_core_pullUpNode(oNode) {
	var oParent = oNode.parentNode;
	
	var oPrev  = oParent.cloneNode();
	var oNext  = oParent.cloneNode();

	while (oParent.childNodes.length) {
		if (oParent.childNodes.item(0) == oNode)
			break;
		oPrev.appendChild(oParent.childNodes.item(0));
	}
	
	while (oParent.childNodes.length >= 2) 
		oNext.appendChild(oParent.childNodes.item(1));
	
	if (0 != SyndLib.trim(oPrev.innerText).length) {
		oParent.replaceNode(oPrev);
		oPrev.insertAdjacentElement('afterEnd', oNode);
	}
	else {
		oParent.removeChild(oNode)
		oParent.replaceNode(oNode);
	}
	
	if (0 != SyndLib.trim(oNext.innerText).length) 
		oNode.insertAdjacentElement('afterEnd', oNext);
	
	return oNode;
}

function debug(oNode) {
	var i = 0, sMessage = oNode+"\n";
	for (sProp in oNode) {
		if (++i > 6) {
			sMessage += "\n";
			i = 0;
		}
		sMessage += sProp+" ";
	}
	alert(sMessage);
}


// Environment //////////////////////////////////
function srte_env_get(key) {
	var iStartIndex = document.cookie.indexOf(key+'=');
	if (-1 == iStartIndex) 
		return null;

	iStartIndex += key.length+1;
	var iEndIndex = document.cookie.indexOf(';', iStartIndex);
	if (-1 == iEndIndex) 
		iEndIndex = document.cookie.length;
	return unescape(document.cookie.substring(iStartIndex, iEndIndex));
}

function srte_env_set(key, value) {
	var date = new Date();
	date.setFullYear(date.getFullYear()+1);
	document.cookie = key+'='+escape(value)+'; path=/; expires='+date.toGMTString()+';';
}
/////////////////////////////////////////////////