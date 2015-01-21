using System;
using System.Collections.Generic;
using System.Text;
using System.Net.Mail;

namespace Inventory {
	public interface ITransport {
		void Send(Inventory.DeviceInfo.Component device);
	}
	
	public class SoapTransport : ITransport {
		private IInventoryModule Inventory;
		private XmlFormatter Formatter;
		
		public SoapTransport(IInventoryModule inventory, XmlFormatter formatter) {
			Inventory = inventory;
			Formatter = formatter;
		}

		public void Send(Inventory.DeviceInfo.Component device) {
			Inventory.agent(Formatter.ToString(device));
		}
	}
	
	public class MailTransport : ITransport {
		private string SmtpServer;
		private string EmailAddress;
		private XmlFormatter Formatter;
		
		public MailTransport(string server, string address, XmlFormatter formatter) {
			SmtpServer = server;
			EmailAddress = address;
			Formatter = formatter;
		}
		
		public void Send(Inventory.DeviceInfo.Component device) {
			SmtpClient client = new SmtpClient(SmtpServer);
			MailMessage message = new MailMessage(
				"noreply@example.com", EmailAddress,
				"Inventory Agent at '" + Environment.MachineName + "'",
				Formatter.ToString(device));
			message.BodyEncoding = Encoding.UTF8;
			client.Send(message);			
		}
	}
}
