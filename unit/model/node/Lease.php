<?php
require_once 'unit/SyndNodeTestCase.class.inc';
require_once 'core/lib/rpc/RpcTransport.class.inc';

class _model_Node_Lease extends SyndNodeTestCase {
	function testServiceLevelAgreement() {
		$folder = SyndNodeLib::factory('folder');
		$folder->setTitle('_unit_test: This folder is safe to delete');
		$lease = $folder->appendChild($folder->_storage->factory('lease'));
		$lease->data['CLIENT_NODE_ID'] = 'user_null.null';
		
		$descriptions = $lease->getServiceLevelDescriptions();
		$this->assertEquals(0, count($descriptions));

		$sld1 = $folder->appendChild($folder->_storage->factory('sld'));
		$sld1->setTitle('_unit_test: This SLD is safe to delete');
		$sld2 = $folder->appendChild($folder->_storage->factory('sld'));
		$sld2->setTitle('_unit_test: This SLD is safe to delete');
		
		$lease->appendChild($sld1);
		$lease->merge(array('sld' => $sld2->nodeId));
		$descriptions = $lease->getServiceLevelDescriptions();
		$this->assertEquals(2, count($descriptions));
		
		// Test flush and reload
		$folder->save();
		$sld1->save();
		$sld2->save();
		$lease->save();
		$folder->flush();
		
		$persistent = $folder->_storage->getPersistentStorage();
		$lease2 = $persistent->getInstance($lease->nodeId);
		$descriptions = $lease2->getServiceLevelDescriptions();
		$this->assertEquals(2, count($descriptions));
		
		// Test serialize
		$lease3 = unserialize(serialize($lease));
		$descriptions = $lease3->getServiceLevelDescriptions();
		$this->assertEquals(2, count($descriptions));
		
		// Test remove
		$lease->removeChild($sld2);
		$descriptions = $lease->getServiceLevelDescriptions();
		$this->assertEquals(1, count($descriptions));
		
		$lease->save();
		$lease->flush();
		$lease4 = $persistent->getInstance($lease->nodeId);
		$descriptions = $lease4->getServiceLevelDescriptions();
		$this->assertEquals(1, count($descriptions));
		
		$sql = "
			SELECT COUNT(*) FROM synd_inv_lease_sld ls
			WHERE ls.lease_node_id = ".$lease->_db->quote($lease->nodeId);
		$this->assertEquals(2, $lease->_db->getOne($sql));

		// Cleanup
		$lease->delete();
		$sld1->delete();
		$sld2->delete();
		$folder->delete();
	}
	
	function testRemote() {
		global $synd_user;
		$node = SyndNodeLib::factory('lease');
		$node->setParent(SyndNodeLib::getInstance('case._unit_test'));
		$node->data['CLIENT_NODE_ID'] = $synd_user->nodeId;
		$node->save();
		$node->flush();
		
		$xmlrpc = Module::getInstance('xmlrpc');
		$remote = Activator::getInstance($xmlrpc->getEndpoint($node));
		
		$this->assertFalse(empty($node->nodeId));
		$this->assertEquals($node->nodeId, $remote->nodeId);
		$this->assertEquals($node->nodeId(), $remote->nodeId());
		
		$node->delete();
	}
}
