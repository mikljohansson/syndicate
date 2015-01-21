function synd_cookie_set(key, value) {
	document.cookie = key+'='+escape(value)+'; path=/;';
}

function synd_radio_play(base, uri, title) {
	var oFrame = window.parent.document.getElementById('synd_radio_frame');
	if (null == oFrame) {
		synd_cookie_set('synd[radio][page]', window.location);
		synd_cookie_set('synd[radio][uri]', uri);
		synd_cookie_set('synd[radio][title]', title);
		window.location = base;
	}
	else {
		var sBase = base+'player/';
		sBase += (-1 == sBase.indexOf('?') ? '?' : '&');
		oFrame.src = sBase+'synd[radio][uri]='+uri+'&synd[radio][title]='+title;
	}
}