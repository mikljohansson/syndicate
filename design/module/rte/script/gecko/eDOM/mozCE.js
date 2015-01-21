/* ***** BEGIN LICENSE BLOCK *****

Selection.prototype.toggleAttribute = function(attribute, attrValue, attrDefault) {
	if(current==attrValue) {
		if(attrDefault=="") {			// if the default attribute value is an empty string, then remove the attribute
			contextNode.removeAttribute(attribute);
		}
		else {										// otherwise set it to the default
			contextNode.setAttribute(attribute, attrDefault);
		}
	}
	else{
		contextNode.setAttribute(attribute, attrValue);
	}
}