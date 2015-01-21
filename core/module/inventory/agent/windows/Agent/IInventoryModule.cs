using System;

namespace Inventory {
	/**
	 * Inventory module interface
	 *
	 * The inventory module exposes this interface via SOAP and
	 * XMLRPC at:
	 *
	 * <pre>
	 *  http://www.example.com/synd/soap/inventory/
	 *  http://www.example.com/synd/xmlrpc/inventory/
	 * </pre>
	 *
	 * @access		public
	 * @package		synd.core.module
	 */
	public interface IInventoryModule {
		/**
		 * Batch update a computer instance
		 *
		 * Returns the address of the computer instance updated or 
		 * created by this call.
		 *
		 * @link	http://svn.synd.info/synd/branches/php4/core/module/inventory/agent/device.dtd
		 * @param	string	Computer information
		 * @return	bool
		 */
		bool agent(string xml);
	}
}
