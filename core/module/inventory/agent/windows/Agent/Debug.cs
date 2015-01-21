using System;
using System.Xml;

namespace Inventory {
	public class Debug : Service {
		/**
		 * Console application entry point for debugging purposes
		 */
		static void Main(string[] args) {
			string path = System.Reflection.Assembly.GetExecutingAssembly().Location;
			path = path.Substring(0, path.LastIndexOf("\\"));
			
			XmlTextReader xmlReader = new XmlTextReader(path+"\\Inventory.xml");
			xmlReader.WhitespaceHandling = WhitespaceHandling.None;
			XmlDocument config = new XmlDocument();
			config.Load(xmlReader);

			Debug service = new Debug();
			service.LoadConfig();
			service.InitializeChannel();

			XmlNode location = config.SelectSingleNode("/Inventory/Agent").Attributes.GetNamedItem("Location");
			IInventoryModule inventory = (IInventoryModule)Activator.GetObject(typeof(IInventoryModule), location.Value);
			Agent agent = new Agent(new SoapTransport(inventory, new XmlFormatter()), config.SelectSingleNode("/Inventory/Agent"));

			//Agent agent = new Agent(new DebugTransport(), config.SelectSingleNode("/Inventory/Agent"));

			agent.Run();
		}
	}
	
	public class DebugTransport : ITransport {
		public void Send(Inventory.DeviceInfo.Component device) {
			XmlFormatter formatter = new XmlFormatter();
			Console.WriteLine(formatter.ToString(device));
		}
	}
}
