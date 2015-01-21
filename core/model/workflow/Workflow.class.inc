<?php
require_once 'core/model/workflow/WorkflowActivity.class.inc';

/**
 * @access		public
 * @package		synd.core.model
 */
interface Workflow {
	/**
	 * Registers a listener in this workflow
	 *
	 * Listeners are removed after they have handled a single event,
	 * should they need to continue listening they must register 
	 * again. The workflow is considered completed when no more
	 * listeners exist.
	 *
	 * @param	WorkflowActivity	The monitoring activity
	 * @param	DomainObject		Domain context to listen on
	 * @param	string				Subject interface to screen for
	 * @param	array				Event interfaces to capture
	 */
	function addListener(WorkflowActivity $activity, DomainObject $context, $interface, $events);
	
	/**
	 * Process a domain object
	 * @param	array		Objects to process
	 * @param	Response	Response to send back to client
	 */
	function process($subjects, Response $response);

	/**
	 * Process a domain event
	 * @param	DomainEvent		Event to process
	 */
	function handleEvent(DomainEvent $event);
}
