/**
 * SyndRTE standalone javascript loader
 *
 * Put this into your page:
 *	<script id="srte_loader" type="text/javascript" src="/path/to/syndrte/loader.js"></script>
 *
 * The loader will take care of the rest, such as loading the main 
 * code, toolbars and stylesheet. It needs the id="srte_loader" to
 * work out the installation path.
 *
 * @access		public
 * @package		synd.rte
 */

var gsToolbars = null;

/**
 * Extracts a cookie
 */
var fpCookie = function(sKey, sDefault) {
	var iStartIndex = document.cookie.indexOf(sKey+'=');
	if (-1 == iStartIndex) 
		return sDefault;

	iStartIndex += sKey.length+1;
	var iEndIndex = document.cookie.indexOf(';', iStartIndex);
	if (-1 == iEndIndex) 
		iEndIndex = document.cookie.length;
	return unescape(document.cookie.substring(iStartIndex, iEndIndex));
}

/**
 * Parses and load the toolbars from text
 */
var fpParseToolbars = function(sText) {
	// SyndRTE base install dir
	sText = sText.replace(/{base}/g, sBase);

	// Position the toolbars
	sText = sText.replace(/{btnLeft}/, fpCookie('srte_toolbar_button[x]', '10px'));
	sText = sText.replace(/{btnTop}/, fpCookie('srte_toolbar_button[y]', '10px'));

	sText = sText.replace(/{propLeft}/, fpCookie('srte_toolbar_properties[x]', '10px'));
	sText = sText.replace(/{propTop}/, fpCookie('srte_toolbar_properties[y]', '45px'));
	sText = sText.replace(/{propVisibility}/, fpCookie('srte_toolbar_properties[vis]', 'hidden'));

	sText = sText.replace(/{domLeft}/, fpCookie('srte_toolbar_dom[x]', '10px'));
	sText = sText.replace(/{domTop}/, fpCookie('srte_toolbar_dom[y]', '95%'));
	sText = sText.replace(/{domVisibility}/, fpCookie('srte_toolbar_dom[vis]', 'hidden'));

	sText = sText.replace(/{imageLeft}/, fpCookie('srte_toolbar_image[x]', '10px'));
	sText = sText.replace(/{imageTop}/, fpCookie('srte_toolbar_image[y]', '100px'));

	return sText;
}

if (document.getElementById) {
	// Get <script> object
	var oLoader = document.getElementById('srte_loader'), httpRequest;

	if (null == oLoader) 
		alert('Error: No <script> element with id="srte_loader"');
	else {
		// Work out the SyndRTE install dir 
		var sBase = oLoader.src.substring(0, oLoader.src.lastIndexOf('/')+1);
		var bToolbarsLoaded = false;
		
		// Callback to initialize the editor when everything is loaded
		var fpLoaderCallback = function() {
			if ('undefined' == typeof(SyndEditor) || null == gsToolbars)
				return;
			// Paste the toolbars into the document
			document.getElementsByTagName('BODY').item(0).
				appendChild(document.createElement('SPAN')).innerHTML = gsToolbars;
			// Initialize the editor
			new SyndEditor(document);
		}
		
		// Include stylesheet
		document.write('<link rel="stylesheet" type="text/css" href="'+sBase+'style.css" />');
		
		// Detect browser and load editor
		if (-1 != navigator.appName.indexOf('Netscape')) {
			// Retrive the toolbars from server
			httpRequest = new XMLHttpRequest();
			httpRequest.open('GET', sBase+'toolbar/toolbar.html', true);
			httpRequest.onreadystatechange = function() {
				if (4 == httpRequest.readyState) {
					gsToolbars = fpParseToolbars(httpRequest.responseText);
					fpLoaderCallback();
				}
			}
			httpRequest.send(null);

			document.write('<script defer="defer" type="text/javascript" src="'+sBase+'script/core.js"></script>');
			document.write('<script defer="defer" type="text/javascript" src="'+sBase+'script/gui.js"></script>');
			document.write('<script defer="defer" type="text/javascript" src="'+sBase+'script/gecko/eDOM/eDOM.js"></script>');
			document.write('<script defer="defer" type="text/javascript" src="'+sBase+'script/gecko/eDOM/eDOMXHTML.js"></script>');
			document.write('<script defer="defer" type="text/javascript" src="'+sBase+'script/gecko/eDOM/domlevel3.js"></script>');
			document.write('<script defer="defer" type="text/javascript" src="'+sBase+'script/gecko/eDOM/mozCE.js"></script>');
			document.write('<script defer="defer" type="text/javascript" src="'+sBase+'script/gecko/Mozile/mozileWrappers.js"></script>');
			document.write('<script defer="defer" type="text/javascript" src="'+sBase+'script/gecko/api.js"></script>');

			// Add onload listener
			window.addEventListener('load', fpLoaderCallback, true);
		}
		else if (-1 != navigator.appName.indexOf('Internet Explorer')) {
			// Retrive the toolbars from server
			try {
				httpRequest = new ActiveXObject('Msxml2.XMLHTTP');
			}
			catch (e) {
				httpRequest = new ActiveXObject('Microsoft.XMLHTTP');
			}

			httpRequest.open('GET', sBase+'toolbar/toolbar.html', true);
			httpRequest.onreadystatechange = function() {
				if (4 == httpRequest.readyState) {
					gsToolbars = fpParseToolbars(httpRequest.responseText);
					fpLoaderCallback();
				}
			}
			httpRequest.send(null);

			document.write('<script defer="defer" type="text/javascript" src="'+sBase+'script/core.js"></script>');
			document.write('<script defer="defer" type="text/javascript" src="'+sBase+'script/gui.js"></script>');
			document.write('<script defer="defer" type="text/javascript" src="'+sBase+'script/ie/api.js"></script>');

			// Add onload listener
			window.attachEvent('onload', fpLoaderCallback);
		}
	}
}
