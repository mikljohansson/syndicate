/**
 * Creates a new calendar
 * @param	Element	The table to attach to
 */
function Calendar(oTable) {
	this._table = oTable;
	oThis = this;
	oTable.attachEvent('onmousedown', function(oEvent) {oThis._callback_onmousedown(oEvent);});
}

/**
 * Attached table
 * @access	private
 */
Calendar.prototype._table = null;

/**
 * Current event
 * @access	private
 */
Calendar.prototype._event = null;

/**
 * Finished creating event callback
 * @access	private
 */
Calendar.prototype._callback = null;

Calendar.prototype.setFinishedCallback = function(fpCallback) {
	this._callback = fpCallback;
}

Calendar.prototype._callback_onmousedown = function(oEvent) {
	if (oEvent.srcElement.getAttribute('row')) {
		var oThis = this, oHtml = document.getElementsByTagName('html').item(0);

		var fpMouseOver = function(oEvent) {
			if (oEvent.srcElement.getAttribute('row')) {
				var sPrev = (oEvent.srcElement.getAttribute('row')-1)+'x'+oEvent.srcElement.getAttribute('col');
				if (oThis._event && document.getElementById(sPrev) && oThis._event.contains(document.getElementById(sPrev))) {
					oThis._event.appendChild(oEvent.srcElement);
				}
				else {
					if (oThis._event)
						oThis._event.destroy();
					oThis._event = new CalendarEvent();
					oThis._event.appendChild(oEvent.srcElement);
				}
			}
			else if (oThis._event) {
				oThis._event.destroy();
				oThis._event = null;
			}
		}		

		var fpMouseUp = function() {
			oHtml.detachEvent('onmouseover', fpMouseOver);
			oHtml.detachEvent('onmouseup', fpMouseUp);
			if (oThis._event && !oThis._event.isFinished && oThis._callback) {
				oThis._callback(oThis._event);
				oThis._event.isFinished = true;
			}
		}

		oHtml.attachEvent('onmouseover', fpMouseOver);
		oHtml.attachEvent('onmouseup', fpMouseUp);

		if (this._event)
			this._event.destroy();
		this._event = new CalendarEvent();
		this._event.appendChild(oEvent.srcElement);
	}
	else if (this._event) {
		this._event.destroy();
		this._event = null;
	}
}

function CalendarEvent(oCell) {
	this._cells = new Object();
}

/**
 * Selected cells
 * @access	private
 */
CalendarEvent.prototype._cells = null;

/**
 * Event is finished (callback has been run)
 * @access	private
 */
CalendarEvent.prototype.isFinished = false;

CalendarEvent.prototype.appendChild = function(oCell) {
	this._cells[oCell.id] = oCell;
	oCell.className = 'Event TentativeEvent';
}

CalendarEvent.prototype.removeChild = function(oCell) {
	delete this._cells[oCell.id];
	oCell.className = '';
}

CalendarEvent.prototype.destroy = function() {
	for (key in this._cells)
		this._cells[key].className = '';
}

CalendarEvent.prototype.contains = function(oCell) {
	return undefined != this._cells[oCell.id];
}

/**
 * Returns the start of the event
 * @param	integer	Resolution in seconds per row
 * @return	Date
 */
CalendarEvent.prototype.getStart = function() {
	var oDate = null, oCurrent;
	for (key in this._cells) {
		if (this._cells[key].getAttribute) {
			oCurrent = new Date(new Number(this._cells[key].getAttribute('start')) * 1000);
			if (null == oDate || oDate > oCurrent)
				oDate = oCurrent;
		}
	}
	return oDate;
}

/**
 * Returns the end of the event
 * @param	integer	Resolution in seconds per row
 * @return	Date
 */
CalendarEvent.prototype.getEnd = function() {
	var oDate = null, oCurrent;
	for (key in this._cells) {
		if (this._cells[key].getAttribute) {
			oCurrent = new Date(new Number(this._cells[key].getAttribute('end')) * 1000);
			if (null == oDate || oCurrent)
				oDate = oCurrent;
		}
	}
	return oDate;
}

if (!window.attachEvent) {
	Element.prototype.attachEvent = function(sEvent, fpCallback) {
		this.addEventListener(sEvent.substring(2), fpCallback, false);
	}

	Element.prototype.detachEvent = function(sEvent, fpCallback) {
		this.removeEventListener(sEvent.substring(2), fpCallback, false);
	}

	Window.prototype.attachEvent = Element.prototype.attachEvent;
	Window.prototype.detachEvent = Element.prototype.detachEvent;

	Event.prototype.__defineGetter__(
		'srcElement', 
		function() {
			return this.target;
		}
	);

	Event.prototype.__defineSetter__(
		'cancelBubble', 
		function(bCancel) { 
			if (true == bCancel) {
				this.stopPropagation();
				this.preventDefault();
			}
		}
	);

	KeyboardEvent.prototype.__defineSetter__(
		'cancelBubble', 
		function(bCancel) { 
			if (true == bCancel) {
				this.stopPropagation();
				this.preventDefault();
			}
		}
	);
}
