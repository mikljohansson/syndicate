using System;
using System.ServiceProcess;
using System.Configuration.Install;

/**
 * Inventory service installer
 *
 * @access		public
 * @package		synd.core.module
 */
namespace Inventory {
	public class InventoryInstaller : Installer {
		public InventoryInstaller(string name) {
			ServiceProcessInstaller procInstaller = new ServiceProcessInstaller();

			// Run as NetworkService
			procInstaller.Account = ServiceAccount.NetworkService;

			ServiceInstaller servInstaller = new ServiceInstaller();
			servInstaller.ServiceName = name;
			servInstaller.StartType = ServiceStartMode.Automatic;

			this.Installers.Add(procInstaller);
			this.Installers.Add(servInstaller);		
		}
	}
}
