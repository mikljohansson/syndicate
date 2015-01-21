<?php
require_once 'core/model/node/issue.class.inc';

/**
 * Invoice issue
 */
class synd_node_invoice extends synd_node_issue /* implements ISyndPrintable */ {
	/**
	 * Buffered ids of leasings
	 * @see		synd_node_used
	 * @access	private
	 */
	var $_leasings = null;
	
	function __factory() {
		global $synd_user;
		parent::__factory();

		$this->setCreator($synd_user);
		$this->setAssigned($synd_user);

		$this->data['INFO_HEAD'] = SyndLib::translate('Invoice');
	}
	
	function __clone() {
		parent::__clone();
		$this->data['INFO_INVOICE_NUMBER'] = null;
	}
	
	static function getEntityQuery(DomainStorage $storage) {
		$query = parent::getEntityQuery($storage);
		$query->join('synd_issue_invoice');
		return $query;
	}

	function hasPermission($user, $perm, $arg = null) {
		if ('print' == $perm)
			return $this->hasPermission($user, 'admin');
		if ('issue_client' == $perm)
			return $this->getParent()->hasPermission($user, 'admin');
		
		// Restrict write access for clients
		if ('write' == $perm)
			return $this->hasPermission($user, 'admin');
		
		if ('merge' == $perm && in_array($arg, array(
			'RECEIPT_NODE_ID','INFO_AMOUNT_TAXED','INFO_AMOUNT_UNTAXED')))
			return $this->hasPermission($user, 'write');
		return parent::hasPermission($user, $perm, $arg);
	}

	/**
	 * Traps append of leasings to create relations
	 */
	function appendChild($child) {
		if (($child instanceof synd_node_used)) {
			if (null === $this->_leasings)
				$this->getLeasings();
			if (!in_array($child->nodeId, $this->_leasings)) {
				$this->_leasings[] = $child->nodeId;
				$this->_node_onchange_internal();
			}
			return $child;
		}
		return parent::appendChild($child);
	}
	
	/**
	 * Returns the OCR number for this invoice
	 *
	 * The OCR number if based on the social security number of the
	 * invoice client, the date the invoice is due and a parity 
	 * number according to the 10-modulo method.
	 *
	 * @link	http://www.bankgirot.se/upload/Gemensamt/Trycksaker/Manualer/10-MODUL.pdf
	 * @return	integer
	 */
	function getInvoiceNumber() {
		if (null == $this->data['INFO_INVOICE_NUMBER'] && null !== ($client = $this->getCustomer()) && !$client->isNull()) {
			$ssn = is_numeric($client->getSocialSecurityNumber()) ? 
				$client->getSocialSecurityNumber() : crc32($client->nodeId);

			$semester = date('n') < 8 ? 1 : 2;
			$ssnPart = sprintf('%06u', substr($ssn,0,6)).substr(date('y'),-1).$semester;
			$unique = $this->_db->nextId('synd_issue_invoice') % $ssnPart % 10;
			
			for ($i=0; null == $this->data['INFO_INVOICE_NUMBER'] && $i<1000; $i++, $unique++) {
				$ocr = $ssnPart.$unique.((strlen($ssnPart)+strlen($unique)+2) % 10);
				$ocr .= synd_node_invoice::_parity($ocr);

				$sql = "
					SELECT i.node_id FROM synd_issue_invoice i
					WHERE i.info_invoice_number = ".$this->_db->quote($ocr);
				if (null == ($id = $this->_db->getOne($sql)) || $id == $this->nodeId) {
					$this->data['INFO_INVOICE_NUMBER'] = $ocr;
					$this->_save();
				}
			}

			if (null == $this->data['INFO_INVOICE_NUMBER'])
				trigger_error("Failed to allocate an invoice number for '$this->nodeId'", E_USER_WARNING);
		}
		
		return $this->data['INFO_INVOICE_NUMBER'];
	}

	/**
	 * @access	private
	 */
	static function _parity($number) {
		$sum = 0;
		$number = preg_replace('/[^\d]/', '', $number);
		
		for ($i=strlen($number)-1, $j=2; $i>=0; $i--, $j = $j % 2 + 1) {
			for ($k=0, $product = $number{$i} * $j; $k<strlen($product); $k++)
				$sum += substr($product,$k,1);
		}

		return (10 - $sum % 10) % 10;
	}
	
	/**
	 * Allocate an invoice number if not set
	 */
	function setCustomer($client) {
		parent::setCustomer($client);
		if (null == $this->data['INFO_INVOICE_NUMBER'])
			$this->getInvoiceNumber();
	}

	/**
	 * Specifies the lease to couple this invoice to
	 * @param	synd_node_lease	The lease to assign
	 */
	function setLease($lease) {
		$this->setCustomer($lease->getCustomer());
		$this->data['LEASE_NODE_ID'] = $lease->nodeId;
	}

	/**
	 * Returns the lease this invoice is associated with
	 * @return	synd_node_lease	Returns null if no lease found
	 */
	function getLease() {
		return $this->_storage->getInstance($this->data['LEASE_NODE_ID']);
	}

	/**
	 * @return	Collection
	 */
	static function resolveInvoiceNumbers($numbers) {
		if (empty($numbers))
			return null;
		
		$storage = SyndNodeLib::getDefaultStorage('invoice');
		$database = $storage->getDatabase();
		
		$sql = "
			SELECT ii.node_id FROM synd_issue_invoice ii
			WHERE LOWER(ii.info_invoice_number) IN (".implode(',',$database->quote(array_map('strtolower', $numbers))).")";
		return SyndType::factory('query_collection', $storage, $sql);
	}

	/**
	 * Returns the total amount including taxes
	 * @return	integer
	 */
	function getAmount() {
		return $this->data['INFO_AMOUNT_TAXED'];
	}

	/**
	 * Returns the total amount excluding taxes
	 * @return	integer
	 */
	function getUntaxedAmount() {
		return $this->data['INFO_AMOUNT_UNTAXED'];
	}
	
	function getDue() {
		return $this->getResolveBy();
	}

	/**
	 * Returns the timestamp when this invoice was paid
	 * @return	integer
	 */
	function getPaid() {
		return $this->data['TS_PAID'];
	}

	/**
	 * Returns true if this invoice has been paid
	 * @return	bool
	 */
	function isPaid() {
		return !empty($this->data['TS_PAID']);
	}
	
	/**
	 * Mark invoice as paid
	 * @param	integer	Timestamp of payment
	 */
	function setPaid($ts = null) {
		$this->setStatus(synd_node_issue::CLOSED);
		if (empty($this->data['TS_PAID'])) {
			$this->data['TS_PAID'] = null != $ts ? $ts : time();
			$this->_save();
		}
	}

	/**
	 * Adds the invoice number, amount and paid date to the index
	 */
	function getIndexDocument($index) {
		if (null == ($document = parent::getIndexDocument($index)))
			return null;
		$fragments = array(
			$index->createFragment($this->data['INFO_INVOICE_NUMBER'], 'id'),
			$index->createFragment(date('Y-m-d', $this->data['TS_PAID'])),
			$document);
		return $index->createComposite($fragments, null, 1, $this->data['TS_UPDATE']);
	}

	/**
	 * Returns leasings couple to this issue
	 * @see		synd_node_used
	 * @return	array
	 */
	function getLeasings() {
		if (null === $this->_leasings) {
			$sql = "
				SELECT il.leasing_node_id FROM synd_issue_leasing il
				WHERE il.issue_node_id = ".$this->_db->quote($this->nodeId);
			$this->_leasings = (array)$this->_db->getCol($sql);
			$this->_node_onchange_internal();
		}
		return $this->_storage->getInstances($this->_leasings);
	}
	
	function getCompositeData() {
		$data = parent::getCompositeData();
		$data['paid'] = (int)!empty($data['TS_PAID']);
		return $data;
	}

	function merge($data) {
		if (isset($data['INFO_STATUS'])) {
			if (empty($data['paid']))
				$data['TS_PAID'] = null;
			else if (empty($this->data['TS_PAID']))
				$this->setPaid();
		}
		parent::merge($data);
	}

	function toPrinter($printer) {
		$template = $this->getReceiptTemplate();
		if ($template->isNull()) 
			return false;
		require_once 'core/lib/SyndPrint.class.inc';
		return SyndPrint::pdfPrint($printer, $template->path(), $this->getReceiptFilter());
	}

	function printReceipt() {
		$inventory = Module::getInstance('inventory');
		if (null != ($printer = $inventory->getPrinter())) 
			return $this->toPrinter($printer);
		return false;
	}
	
	function getReceiptTemplate() {
		if (null !== ($template = $this->_storage->getInstance($this->data['RECEIPT_NODE_ID'])))
			return $template;
		$inventory = Module::getInstance('inventory');
		$templates = $inventory->getInheritedTemplates(get_class($this));
		if (!empty($templates))
			return $templates[key($templates)];
		return $template = SyndNodeLib::getInstance('null.null');
	}

	function getReceiptOptions() {
		$inventory = Module::getInstance('inventory');
		return $inventory->getInheritedTemplates(get_class($this));
	}
	
	function getReceiptFilter() {
		$client = $this->getCustomer();
		$assigned = $this->getAssigned();
		
		$address = $client->getAddress();
		$addr = $client->toString();
		if (!empty($address['CO']))
			$addr .= "\r\n".$address['CO'];
		if (!empty($address['STREET']))
			$addr .= "\r\n".$address['STREET'];
		if (!empty($address['ZIP']) && !empty($address['CITY']))
			$addr .= "\r\n".$address['ZIP'].' '.$address['CITY'];
		if (!empty($address['COUNTRY']))
			$addr .= "\r\n".$address['COUNTRY'];
		
		return array_merge($this->data, array(
			'ID'		=> $this->objectId(),
			'DATE'		=> date('Y-m-d'), 
			'CREATED'	=> date('Y-m-d', $this->data['TS_CREATE']), 
			'DUE'		=> date('Y-m-d', $this->getDue()),
			'CUSTOMER'		=> array(
				'CAPTION'		=> null != $client->getContact() ? $client->toString().' ('.$client->getContact().')' : $client->toString(),
				'NAME'			=> $client->toString(),
				'PHONE'			=> $client->getPhone(),
				'ADDRESS'		=> $addr),
			'ASSIGNED'	=> array(
				'CAPTION'		=> null != $assigned->getContact() ? $assigned->toString().' ('.$assigned->getContact().')' : $assigned->toString(),
				'NAME'			=> $assigned->toString(),
				'PHONE'			=> $assigned->getPhone()),
			'INVOICE'	=> array(
				'NUMBER'	=> $this->getInvoiceNumber(),
				'UNTAXED'	=> $this->getUntaxedAmount(), 
				'TAXED'		=> $this->getAmount(), 
				'VAT'		=> $this->getAmount() - $this->getUntaxedAmount()),
			'ITEM'		=> array(
				array(
					'CAPTION'	=> $this->getTitle(),
					'QUANTITY'	=> 1,
					'AT'		=> $this->getUntaxedAmount(),
					'AMOUNT'	=> $this->getUntaxedAmount()),
				),
			));
	}
	
	/**
	 * @see		printReceipt()
	 */
	function _view_printInvoice(Request $request, Template $page) {
		if (!$this->isPermitted('print'))
			throw new ForbiddenException();
		$this->printInvoice();
	}
	
	/**
	 * Generates and pipes an PDF to browser
	 * @param	string	template=node.file.123	Render using a specific template
	 */
	function _view_invoice(Request $request, Template $page) {
		require_once 'core/lib/SyndPrint.class.inc';
		if (!$this->isPermitted('read') && (empty($request[2]) || !$this->validateAuthenticationToken($request[2])))
			throw new ForbiddenException();
		if (empty($request['template']) || null == ($receipt = SyndLib::getInstance($request['template'])))
			$receipt = $this->getReceiptTemplate();
		if (!$receipt->isNull()) {
			header('Content-Type: application/pdf');
			header('Content-Disposition: attachment; filename="'.str_replace('"','',$this->getInvoiceNumber()).'.pdf"');
			return array('content' => SyndPrint::pdfMerge($receipt->path(), $this->getReceiptFilter()), 'page' => null);
		}
	}

	/**
	 * Returns the absolute uri to the invoice
	 * @return	string
	 */
	function getInvoiceUri() {
		return tpl_request_host().rtrim(tpl_view('inventory','invoke',$this->nodeId,'invoice',$this->getAuthenticationToken(),$this->getInvoiceNumber().'.pdf'),'/');
	}

	/**
	 * Revoke payment of this invoice
	 */
	function _view_revoke(Request $request, Template $page) {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		if (!isset($request['confirm']))
			return array('content' => $page->fetchnode($this,'confirm_revoke_payment.tpl'));

		$this->data['TS_PAID'] = null;
		$this->setStatus(self::PENDING);
		$this->_save();
	}

	/**
	 * Syncronize assigned leasings
	 */
	function _entity_onpostflush() {
		parent::_entity_onpostflush();
		
		if (null !== $this->_leasings) {
			$sql = "
				SELECT il.leasing_node_id FROM synd_issue_leasing il
				WHERE il.issue_node_id = ".$this->_db->quote($this->nodeId);
			$leasings = (array)$this->_db->getCol($sql);
			
			if (count($remove = array_diff($leasings, $this->_leasings))) {
				$sql = "
					DELETE FROM synd_issue_leasing
					WHERE 
						issue_node_id = ".$this->_db->quote($this->nodeId)." AND
						leasing_node_id IN (".implode(',',$this->_db->quote($remove)).")";
				if (SyndLib::isError($this->_db->query($sql)))
					return false;
			}

			if (count($append = array_diff($this->_leasings, $leasings))) {
				foreach ($append as $id) {
					$sql = "
						INSERT INTO synd_issue_leasing
						(issue_node_id, leasing_node_id)
						VALUES
						(".$this->_db->quote($this->nodeId).",
						 ".$this->_db->quote($id).")";
					if (SyndLib::isError($this->_db->query($sql)))
						return false;
				}
			}
		}
	}
}
