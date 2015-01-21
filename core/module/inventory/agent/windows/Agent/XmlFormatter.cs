using System;
using System.Xml;
using System.Text;
using System.Collections;
using Inventory.DeviceInfo;

namespace Inventory {
	public class XmlFormatter : ComponentVisitor {
		private XmlTextWriter _document = null;
		
		public XmlFormatter() {
		}

		public string ToString(Component device) {
			StringBuilder buffer = new StringBuilder();
			_document = new XmlTextWriter(new System.IO.StringWriter(buffer));
			//_document.Formatting = Formatting.Indented;
			_document.WriteStartDocument();
			_document.WriteDocType("device", "-//Synd//DTD Device 1.0//EN", "http://svn.synd.info/synd/branches/php4/core/module/inventory/agent/device.dtd", null);

			device.AcceptVisitor(this);

			_document.WriteEndDocument();
			_document.Close();
			_document = null;
			return buffer.ToString();
		}
		
		public void Visit(Device device, ArrayList children) {
			_document.WriteStartElement("device");
			_document.WriteAttributeString("xmlns", "http://www.synd.info/2005/device");
			base.Visit(device, children);
			_document.WriteEndElement();
		}

		public void Visit(Listener device) {
			_document.WriteStartElement("listener");
			_document.WriteAttributeString("method", "soap");
			_document.WriteAttributeString("version", device.Version);
			_document.WriteAttributeString("action", device.Action);
			_document.WriteString(device.Uri);
			_document.WriteEndElement();
		}

		public void Visit(MotherBoard device, ArrayList children) {
			_document.WriteStartElement("motherboard");
			_document.WriteAttributeString("vendor", device.Vendor);
			_document.WriteAttributeString("version", device.Version);
			_document.WriteAttributeString("release", device.Release);
			_document.WriteAttributeString("serial", device.Serial);
			base.Visit(device, children);
			_document.WriteEndElement();
		}

		public void Visit(BIOS device) {
			_document.WriteStartElement("bios");
			_document.WriteAttributeString("vendor", device.Vendor);
			_document.WriteAttributeString("version", device.Version);
			_document.WriteAttributeString("release", device.Release);
			_document.WriteEndElement();
		}

		public void Visit(RAM device) {
			_document.WriteStartElement("ram");
			_document.WriteAttributeString("name", device.Slot);
			_document.WriteAttributeString("type", device.Type);
			_document.WriteAttributeString("bank", device.Bank);
			_document.WriteAttributeString("size", device.Size.ToString());
			_document.WriteAttributeString("frequency", device.Frequency.ToString());
			_document.WriteEndElement();
		}

		public void Visit(Port device) {
			_document.WriteStartElement("port");
			_document.WriteAttributeString("name", device.Name);
			_document.WriteAttributeString("type", device.Type);
			_document.WriteAttributeString("connector", device.Connector);
			_document.WriteEndElement();
		}

		public void Visit(Slot device) {
			_document.WriteStartElement("slot");
			_document.WriteAttributeString("name", device.Name);
			_document.WriteAttributeString("type", device.Type);
			_document.WriteAttributeString("occupied", device.Occupied ? "1" : "0");
			_document.WriteEndElement();
		}

		public void Visit(Processor device) {
			_document.WriteStartElement("cpu");
			_document.WriteAttributeString("vendor", device.Vendor);
			_document.WriteAttributeString("version", device.Version);
			_document.WriteAttributeString("frequency", device.Frequency.ToString());
			_document.WriteAttributeString("serial", device.Serial);
			_document.WriteEndElement();
		}
		
		public void Visit(Keyboard device) {
			_document.WriteStartElement("keyboard");
			_document.WriteString(device.Version);
			_document.WriteEndElement();
		}

		public void Visit(PointingDevice device) {
			_document.WriteStartElement("pointingdevice");
			_document.WriteAttributeString("vendor", device.Vendor);
			_document.WriteString(device.Version);
			_document.WriteEndElement();
		}

		public void Visit(Disk device, ArrayList children) {
			_document.WriteStartElement("disk");
			_document.WriteAttributeString("name", device.Name);
			_document.WriteAttributeString("version", device.Version);
			_document.WriteAttributeString("size", device.Size.ToString());
			_document.WriteAttributeString("cache", device.Cache.ToString());
			base.Visit(device, children);
			_document.WriteEndElement();
		}

		public void Visit(Partition device) {
			_document.WriteStartElement("partition");
			_document.WriteAttributeString("name", device.Name);
			_document.WriteAttributeString("size", device.Size.ToString());
			_document.WriteAttributeString("type", device.Type);
			_document.WriteEndElement();
		}
	
		public void Visit(ROMDrive device) {
			_document.WriteStartElement("rom");
			_document.WriteString(device.Version);
			_document.WriteEndElement();
		}

		public void Visit(NetworkDevice device, ArrayList children) {
			_document.WriteStartElement("nic");
			_document.WriteAttributeString("mac", device.MAC);
			_document.WriteAttributeString("version", device.Version);
			base.Visit(device, children);
			_document.WriteEndElement();
		}

		public void Visit(NetworkInterface device) {
			_document.WriteStartElement("interface");
			_document.WriteAttributeString("ip", device.IP);
			_document.WriteAttributeString("netmask", device.Netmask);
			_document.WriteEndElement();
		}

		public void Visit(VideoCard device) {
			_document.WriteStartElement("videocard");
			_document.WriteAttributeString("version", device.Version);
			_document.WriteAttributeString("ram", device.RAM.ToString());
			_document.WriteAttributeString("hres", device.HorizontalResolution.ToString());
			_document.WriteAttributeString("vres", device.VerticalResolution.ToString());
			_document.WriteAttributeString("bits", device.BitsPerPixel.ToString());
			_document.WriteAttributeString("frequency", device.Frequency.ToString());
			_document.WriteEndElement();
		}

		public void Visit(SoundCard device) {
			_document.WriteStartElement("soundcard");
			_document.WriteAttributeString("version", device.Version);
			_document.WriteEndElement();
		}

		public void Visit(Monitor device) {
			_document.WriteStartElement("monitor");
			_document.WriteAttributeString("vendor", device.Vendor);
			_document.WriteAttributeString("version", device.Version);
			_document.WriteAttributeString("size", Math.Round(device.Size, 1).ToString());
			_document.WriteEndElement();
		}

		public void Visit(Inventory.DeviceInfo.OperatingSystem device, ArrayList children) {
			_document.WriteStartElement("os");
			_document.WriteAttributeString("name", device.Name);
			_document.WriteAttributeString("vendor", device.Vendor);
			_document.WriteAttributeString("version", device.Version);
			_document.WriteAttributeString("release", device.Release);
			_document.WriteAttributeString("machinename", device.MachineName);
			_document.WriteAttributeString("serial", device.SerialNumber);
			_document.WriteAttributeString("path", device.Path);
			base.Visit(device, children);
			_document.WriteEndElement();
		}

		public void Visit(Printer device) {
			_document.WriteStartElement("printer");
			_document.WriteAttributeString("name", device.Name);
			_document.WriteAttributeString("driver", device.Driver);
			_document.WriteAttributeString("port", device.Port);
			_document.WriteEndElement();
		}

		public void Visit(Product device) {
			_document.WriteStartElement("product");
			_document.WriteAttributeString("vendor", device.Vendor);
			_document.WriteAttributeString("name", device.Name);
			_document.WriteAttributeString("version", device.Version);
			_document.WriteAttributeString("installdate", device.InstallDate);
			_document.WriteAttributeString("state", device.State ? "1" : "0");
			_document.WriteEndElement();
		}

		public void Visit(Image device) {
			_document.WriteStartElement("image");
			_document.WriteString(device.Version);
			_document.WriteEndElement();
		}
	}
}
