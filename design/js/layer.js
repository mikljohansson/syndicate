var goLastLayer = null;

function synd_tooltip_show(oNode, sId) {
	if (!document.getElementById || null == (oLayer = document.getElementById(sId)))
		return;
	if ('block' == oLayer.style.display)
		return;
		
	oLayer.style.display = 'block';
	oLayer.style.width = '300px';

	var iX = null, iY = null;
	if (undefined != oNode.x) {
		iX = oNode.x;
		iY = oNode.y;
	}
	else if (undefined != oNode.clientLeft) {
		iX = event.x;
		iY = event.y;
	}
		
	if (null != iX) {
		var iLeft = iX + oNode.width;
		if (iLeft + 300 > document.body.clientWidth)
			iLeft -= iLeft + 300 - document.body.clientWidth + 10;
		
		oLayer.style.left = iLeft + 'px';
		oLayer.style.top = iY + oNode.height + 'px';

		if (-1 != navigator.appName.indexOf('Internet Explorer')) {
			oLayer.oFrame = document.createElement('iframe');
			oLayer.oFrame.style.position = 'absolute';
			oLayer.oFrame.style.filter='progid:DXImageTransform.Microsoft.Alpha(style=0,opacity=0)';
			oLayer.oFrame.style.height = oLayer.clientHeight + 2 + 'px';
			oLayer.oFrame.style.width = oLayer.clientWidth + 'px';
			oLayer.oFrame.style.left = oLayer.style.left;
			oLayer.oFrame.style.top = oLayer.style.top;
			oLayer.parentNode.insertBefore(oLayer.oFrame, oLayer);
		}
	}
	
	if (null != goLastLayer)
		goLastLayer.style.display = 'none';
	goLastLayer = oLayer;

	oNode.onmouseout = function(oEvent) {
		if (!oEvent || oEvent.relatedTarget) {
			oLayer.style.display = 'none';
			if (undefined != oLayer.oFrame)
				oLayer.oFrame.parentNode.removeChild(oLayer.oFrame);
			goLastLayer = null;
		}
	}
}
