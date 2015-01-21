//The following are for browsers like NS4 or IE5Mac which don't support either
//attachEvent or addEventListener
function MyAttachEvent(obj,evt,fnc){
	if (!obj.myEvents) obj.myEvents={};
	if (!obj.myEvents[evt]) obj.myEvents[evt]=[];
	var evts = obj.myEvents[evt];
	evts[evts.length]=fnc;
}
function MyFireEvent(obj,evt){
	if (!obj || !obj.myEvents || !obj.myEvents[evt]) return;
	var evts = obj.myEvents[evt];
	for (var i=0,len=evts.length;i<len;i++) evts[i]();
}

function _synd_event_cancel(oEvent) {
	if (oEvent.stopPropagation) {
		oEvent.stopPropagation();
		oEvent.preventDefault();
	}
	else {
		oEvent.cancelBubble = true;
		return false;
	}
}

function _synd_event_attach(oNode, sEvent, fpCallback) {
	if (oNode.addEventListener)
		oNode.addEventListener(sEvent.substring(2), fpCallback, false);
	else if (oNode.attachEvent)
		oNode.attachEvent(sEvent, fpCallback);
	else {
		MyAttachEvent(oNode,sEvent.substring(2),fpCallback);
		oNode[sEvent] = function(){ MyFireEvent(oNode,sEvent.substring(2)) };
	}
}
	
function _synd_event_target(oEvent) {
	if (oEvent.currentTarget)
		return oEvent.currentTarget;
	return oEvent.srcElement;
}

function _synd_form_onload() {
	var oForms = document.getElementsByTagName('form');
	for (var i=0; i<oForms.length; i++)
		_synd_event_attach(oForms.item(i), 'onsubmit', _synd_form_onsubmit);

	var oCounter = null;
	var oTextareas = document.getElementsByTagName('textarea');
	for (var i=0; i<oTextareas.length; i++) {
		with (oTextareas.item(i)) {
			if (null != getAttribute('maxlength')) {
				oTextareas.item(i)._maxlength = getAttribute('maxlength');
				if (value.length > _maxlength)
					oTextareas.item(i).value = value.substring(0, _maxlength)
				if (null != (oCounter = document.getElementById(id+'Counter'))) {
					oCounter.value = _maxlength - value.length;
					oTextareas.item(i)._counter = oCounter;
				}
				_synd_event_attach(oTextareas.item(i), 'onkeyup', _synd_form_maxlength);
			}
		}
	}
}

function _synd_form_maxlength(oEvent) {
	var oNode = _synd_event_target(oEvent);
	if (oNode.value.length >= oNode._maxlength)
		oNode.value = oNode.value.substring(0, oNode._maxlength);
	if (undefined != oNode._counter)
		oNode._counter.value = oNode._maxlength - oNode.value.length;
}

function _synd_form_onsubmit(oEvent) {
	var oNode = _synd_event_target(oEvent);
	if (!synd_form_validate(
		oNode.getElementsByTagName('input'),
		oNode.getElementsByTagName('textarea'), 
		oNode.getElementsByTagName('select'))) 
		return _synd_event_cancel(oEvent);
}

function synd_form_validate() {
	var oList, oStyle; 
	var oMatch, sMatch, sValue;
	var sErrorMsg = '', bIsValid = true
	var oFirstInvalid = null;

	for (var n=0; n<synd_form_validate.arguments.length; n++) {
		oList = synd_form_validate.arguments[n];
		for (var i=0; i<oList.length; i++) {
			if (null == (sMatch = oList[i].getAttribute('match')))
				continue;
			
			if (null != oList[i].getAttribute('depend') && 
				(null == (oNode = document.getElementById(oList[i].getAttribute('depend'))) || '' == oNode.value ||
				('radio' == oNode.type.toLowerCase() || 'checkbox' == oNode.type.toLowerCase()) && !oNode.checked)) {
				continue;
			}

			oMatch = new RegExp(sMatch, 'gmi');
			var bValidElem = null == oList[i].value || null != oList[i].value.match(oMatch);
			
			if (bValidElem && 'select' == oList[i].tagName.toLowerCase()) {
				var oOption = oList[i].options[oList[i].selectedIndex]
				bValidElem = !oOption.disabled;
			}

			if (!bValidElem) {
				if (undefined == oList[i]['preBorderStyle']) {
					if (undefined != oList[i].currentStyle)
						oStyle = oList[i].currentStyle;
					else
						oStyle = oList[i].style;
					oList[i]['preBorderStyle'] = oStyle.borderStyle;
					oList[i]['preBorderWidth'] = oStyle.borderWidth;
				}
				
				oList[i].style.border = '2px solid red';
				if (null == oFirstInvalid)
					oFirstInvalid = oList[i];
				
				bIsValid = false;
				var sMsg = oList[i].getAttribute('message');
				if (null != sMsg) 
					sErrorMsg += sMsg+"\n";
			}
			else if (undefined != oList[i]['preBorderStyle']) {
				oList[i].style.border = oList[i]['preBorderStyle'];
				oList[i].style.borderWidth = oList[i]['preBorderWidth'];
			}
		}
	}
	
	if (sErrorMsg.length) {
		alert(sErrorMsg);
		oFirstInvalid.focus();
	}
	
	return bIsValid;
}

_synd_event_attach(window, 'onload', _synd_form_onload);
