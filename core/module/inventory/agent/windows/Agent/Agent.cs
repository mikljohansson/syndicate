using System;
using System.Xml;

namespace Inventory {
	class Agent {
		private ITransport Transport;
		private XmlNode Config;
		private string ChannelUri;

		public Agent(ITransport transport, XmlNode config) : this(transport, config, null) { }
			
		public Agent(ITransport transport, XmlNode config, string uri) {
			Transport = transport;
			if (null == config)
				throw new Exception("No agent config specified.");
			Config = config;
			ChannelUri = uri;
		}

		public void Run() {
			string id = "Windows";
			if (null != Config.Attributes.GetNamedItem("ImageIdentifier"))
				id = Config.Attributes.GetNamedItem("ImageIdentifier").Value;
			
			Inventory.DeviceInfo.Factory factory = 
				new Inventory.DeviceInfo.RegistryDeviceInfo(
				new Inventory.DeviceInfo.WmiDeviceInfo(id), Config);
		
			Inventory.DeviceInfo.Device device = new Inventory.DeviceInfo.Device();

			if (null != ChannelUri) {
				device.AppendChild(new Inventory.DeviceInfo.Listener(ChannelUri, 
					RemoteService.ApiVersion, RemoteService.SoapAction));
			}

			device.AppendChild(factory.GetMotherBoard());
			device.AppendChild(factory.GetProcessors());
			device.AppendChild(factory.GetKeyboard());
			device.AppendChild(factory.GetPointingDevices());
			device.AppendChild(factory.GetStorageDevices());
			device.AppendChild(factory.GetNetworkDevices());
			device.AppendChild(factory.GetVideoCards());
			device.AppendChild(factory.GetSoundCards());
			device.AppendChild(factory.GetMonitors());
			device.AppendChild(factory.GetOperatingSystem());

			Transport.Send(device);
		}
	}
}
