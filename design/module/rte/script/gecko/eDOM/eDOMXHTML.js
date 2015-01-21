/* ***** BEGIN LICENSE BLOCK *****
 * Apply a link to a selection of text
 */
Range.prototype.wrapText = function(elementName)
{
	// if collapsed then return - works for inline style or block: make editor do work
	if(this.collapsed)
		return;

	// go through all text nodes in the range and link to them unless already set to this link
	var textNodes = this.textNodes;
	for(i=0; i<textNodes.length; i++)
	{
		var textContainer = textNodes[i].parentNode;

		// if selected text is part of a span or a then we need to give it an exclusive parent of its own
		// this would only happen when part of a text node is selected either at the beginning or end of a
		// range or both.
		if((textContainer.childNodes.length > 1) && textContainer.nodeNamed("span"))
		{
				var siblingHolder;

				// leave any nodes before or after this one with their own copy of the container
				if(textNodes[i].previousSibling)
				{
					var siblingHolder = textContainer.cloneNode(false);
					textContainer.parentNode.insertBefore(siblingHolder, textContainer);
					siblingHolder.appendChild(textNodes[i].previousSibling);
				}

				if(textNodes[i].nextSibling)
				{
					var siblingHolder = textContainer.cloneNode(false);
					if(textContainer.nextSibling)
						textContainer.parentNode.insertBefore(siblingHolder, textContainer.nextSibling);
					else
						textContainer.parentNode.appendChild(siblingHolder);
					siblingHolder.appendChild(textNodes[i].nextSibling);
				}
		}

		// from now on, we assume that text has an exclusive A or span parent OR it is inside a container
		// that can have an A inserted into it and around the text.
		if(textContainer.nodeName.toLowerCase() != "a")
		{
			// replace a span with an A
			if(textContainer.nodeNamed("span"))
				textContainer = textContainer.parentNode.replaceChildOnly(textContainer, elementName);
			// insert A inside a non span or A container!
			else
			{
				var linkHolder = documentCreateXHTMLElement(elementName);
				textContainer.insertBefore(linkHolder, textNodes[i]);
				linkHolder.appendChild(textNodes[i]);
				textContainer = linkHolder;		
			}
		}

		textNodes[i] = textContainer.firstChild;
	}

	// normalize A elements [may be a waste - why not normalizeElements at the node level?]
	var normalizeRange = document.createRange();
	normalizeRange.selectNode(this.commonAncestorContainer);
	normalizeRange.normalizeElements(elementName);
	normalizeRange.detach();

	// now normalize text
	this.commonAncestorContainer.parentElement.normalize();
	this.__restoreTextBoundaries();
}