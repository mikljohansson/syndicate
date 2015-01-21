using System;
using System.Xml;
using Microsoft.Win32;

namespace Inventory.DeviceInfo {
	public class RegistryDeviceInfo : Factory {
		private XmlNode _config;
		
		public RegistryDeviceInfo(Factory next, XmlNode config) : base(next) {
			_config = config;
		}

		public override Collection GetOperatingSystem() {
			Collection os = base.GetOperatingSystem();
			
			// Find loaded image from registry
			try {
				XmlNode image = _config.SelectSingleNode("LoadedImage");
				RegistryKey key = Registry.LocalMachine.OpenSubKey(image.Attributes.GetNamedItem("Key").Value);
				os.AppendChild(new Image(key.GetValue(image.Attributes.GetNamedItem("Value").Value).ToString()));
			}
			catch {}

			// Add installed software
			try {
				RegistryKey uninstall = Registry.LocalMachine.OpenSubKey("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall");
				foreach (string key in uninstall.GetSubKeyNames()) {
					RegistryKey product = uninstall.OpenSubKey(key);
					os.AppendChild(new Product(
						product.GetValue("Publisher", "").ToString(), product.GetValue("DisplayName", "").ToString(),
						product.GetValue("DisplayVersion", "").ToString(), product.GetValue("InstallDate", "").ToString(), true));
				}
			}
			catch { }

			return os;
		}
	}
}
