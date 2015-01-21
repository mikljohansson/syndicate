function toggleCheckboxes(node, nodes) {
	var i;
	for (i=0; i<nodes.length; i++) {
		if (nodes.item(i).type = node.type)
			nodes.item(i).checked = node.checked;
	}
}

function toggle(id) {
	var node;
	if (document.getElementById && null != (node = document.getElementById(id))) {
		var x = xAbsolute(node.parentNode), html = document.getElementsByTagName('html').item(0), overflow = 0;
		
		if ((overflow = (x + node.clientWidth) - (window.getInnerWidth() + html.scrollLeft)) > 0)
			node.style.left = Math.max(x - overflow - 5, 0) + 'px';
		else
			node.style.left = x + 'px';
			
		if (!node.style.visibility) {
			node.style.visibility = 'visible';
			node.onclick = function() {node.style.visibility = ''};
		}
		else {
			node.style.visibility = '';
			node.onclick = '';
		}
	}
}

if (undefined != window.innerWidth) {
	window.getInnerWidth = function() {return window.innerWidth;}
	window.getInnerHeight = function() {return window.innerHeight;}
}
else {
	window.getInnerWidth = function() {return document.getElementsByTagName('html').item(0).offsetWidth;}
	window.getInnerHeight = function() {return document.getElementsByTagName('html').item(0).offsetHeight;}
}

/**
 * Calculates the absolute x position of an element
 * @access	private
 * @param	Element
 * @return	integer
 */
function xAbsolute(oNode) {
	var pos = oNode.offsetLeft;
	if ('div' == oNode.tagName.toLowerCase() && oNode.scrollLeft)
		pos -= oNode.scrollLeft;
	if (oNode.offsetParent)
		pos += xAbsolute(oNode.offsetParent);
	return pos;
}

/**
 * Calculates the absolute y position of an element
 * @access	private
 * @param	Element
 * @return	integer
 */
function yAbsolute(oNode) {
	var pos = oNode.offsetTop;
	if ('div' == oNode.tagName.toLowerCase() && oNode.scrollTop)
		pos -= oNode.scrollTop;
	if (oNode.offsetParent)
		pos += yAbsolute(oNode.offsetParent);
	return pos;
}
