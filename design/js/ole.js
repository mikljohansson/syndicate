// Adds a new variable
function _synd_ole_create(sName, sValue) {
	oInput = document.createElement('input');
	oInput.type = 'hidden';
	oInput.name = sName;
	oInput.value = sValue;
	return oInput;
}

/**
 * Finds selected ole elements
 */
function _synd_ole_list(bHidden) {
	var oInputs = new Array(), oList = document.getElementsByTagName('input');
	for (var i=0; i<oList.length; i++) {
		if ('selection[]' == oList[i].name && (!bHidden && true == oList[i].checked || bHidden && 'hidden' == oList[i].type))
			oInputs.push(_synd_ole_create(oList[i].name, oList[i].value));
	}
	
	if (oInputs.length)
		oInputs.push(_synd_ole_create('collections[]', 'type.ole_collection.selection'));
	
	return oInputs;
}

/**
 * Finds ole collections
 */
function _synd_ole_collections() {
	var oInputs = new Array(), oList = document.getElementsByTagName('input');
	for (var i=0; i<oList.length; i++) {
		if ('collections[]' == oList[i].name && ('hidden' == oList[i].type || true == oList[i].checked))
			oInputs.push(_synd_ole_create(oList[i].name, oList[i].value));
	}
	return oInputs;
}

/**
 * Calls an uri
 */
function synd_ole_call(sUri, bWarnEmpty) {
	var oForm;
	if (null != (oForm = document.getElementById('synd_ole_form')))
		oForm.parentNode.removeChild(oForm);
	
	var oList = _synd_ole_list();
	if (!oList.length)
		oList = _synd_ole_list(true);
	if (!oList.length)
		oList = _synd_ole_collections();
	
	if (bWarnEmpty && !oList.length)
		return alert('Please make a selection first.');
	
	oForm = document.createElement('form');
	oForm.id = 'synd_ole_form';
	oForm.method = "post";
	oForm.action = sUri;
	oForm.style.margin = "0";

	for (var i=0; i<arguments.length; i++)
		oForm.appendChild(_synd_ole_create(oForm, 'args[]', arguments[i]));

	while (undefined != (oInput = oList.pop()))
		oForm.appendChild(oInput);

	document.getElementsByTagName('body').item(0).appendChild(oForm);
	oForm.submit();
}
