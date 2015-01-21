<?php
/**
 * SyndRTE image upload handler
 *
 * Handles images uploaded from within SyndRTE. The image upload 
 * script is set by specifing the 'upload' attribute on the area 
 * element:
 *
 * <div class="srte_area" id="data[INFO_BODY]" upload="/syndrte/upload.php">
 *
 * The script will recieve a POST varible named 'image' containing 
 * the uploaded image. The 'redirect' variable specifies where to 
 * redirect after storing the image. 
 *
 * Return status is specified by appending either the 'image=' or 
 * 'error=' variable to the redirect uri. Take care to append either
 * '?' or '&' to the uri when setting the variables and urlencode
 * their value properly (use rawurlencode() for php).
 *
 * The image variable should contain the relative location of the 
 * uploaded image, the error variable should be set in case of error
 * and the user will be alert()'ed to its content.
 *
 * @access		public
 * @package		synd.rte
 */

/**
 * Path to store images on server, must be writable by the user the 
 * webserver runs as. On unix this can be accomplished with (assuming 
 * your webserver runs as the 'apache' user):
 *
 *  cd <WEBSERVER DOCUMENT ROOT>
 *  mkdir var && mkdir var/images
 *  chown apache var/images 
 */
$relativePath = '/var/images/'
$absolutePath = $_SERVER['DOCUMENT_ROOT'].$relativePath;

if (null == $_FILES['image']['tmp_name'] || null == $_FILES['image']['name']) 
	$error = 'No image recieved';
else {
	$tmpLocation = $_FILES['image']['tmp_name'];
	$info = pathinfo($_FILES['image']['name']);
	if (!preg_match('/^(jpg|jpeg|gif|png|mng)$/i', $info['extension']) || !is_uploaded_file($tmpLocation))
		$error = "Invalid or unsupported image extension: {$info['extension']}";
	else {
		$name = $_FILES['image']['name'];
		if (file_exists($absolutePath.$name))
			$name = substr($info['basename'],0,strlen($info['basename'])-strlen($info['extension'])).
					'-'.md5(uniqid('')).$info['extension'];
		if (!move_uploaded_file($tmpLocation, $absolutePath.$name))
			$error = "Could not move file to permanent location:\r\n  {$absolutePath}{$name}";
	}
}

$redirect = $_REQUEST['redirect'];
$redirect .= false === strpos($redirect, '?') ? '?' : '&';
$redirect .= isset($error) ? 'error='.rawurlencode($error) : 'image='.rawurlencode($relativePath.$name);
header("Location: $redirect");

?>