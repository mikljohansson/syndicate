using System;
using System.Management;
using Inventory.DeviceInfo;

namespace Inventory.DeviceInfo {
	public class WmiDeviceInfo : Factory {
		private string Identifier;
		
		/**
		 * @param	string	Operating system identifier
		 */
		public WmiDeviceInfo(string id) {
			Identifier = id;
		}

		public override Component GetMotherBoard() {
			foreach (ManagementObject mo in new ManagementClass("Win32_BaseBoard").GetInstances()) {
				MotherBoard motherboard = new MotherBoard(
					mo["Manufacturer"].ToString().Trim(), mo["Product"].ToString().Trim(), 
					mo["Version"].ToString().Trim(), mo["SerialNumber"].ToString().Trim());
				motherboard.AppendChild(GetBIOS());

				// Add RAM slots and modules
				foreach (ManagementObject ram in new ManagementClass("Win32_PhysicalMemory").GetInstances()) {
					string type = "";
					switch (ram["MemoryType"].ToString()) {
						case "2": type = "DRAM"; break;
						case "3": type = "Synchronous DRAM"; break;
						case "4": type = "Cache DRAM"; break;
						case "5": type = "EDO"; break;
						case "6": type = "EDRAM"; break;
						case "7": type = "VRAM"; break;
						case "8": type = "SRAM"; break;
						case "9": type = "RAM"; break;
						case "10": type = "ROM"; break;
						case "11": type = "Flash"; break;
						case "12": type = "EEPROM"; break;
						case "13": type = "FEPROM"; break;
						case "14": type = "EPROM"; break;
						case "15": type = "CDRAM"; break;
						case "16": type = "3DRAM"; break;
						case "17": type = "SDRAM"; break;
						case "18": type = "SGRAM"; break;
						case "19": type = "RDRAM"; break;
						case "20": type = "DDR"; break;
					}
					motherboard.AppendChild(new RAM(
						ram["DeviceLocator"].ToString(), type, ram["BankLabel"].ToString(),
						(int)(Convert.ToInt64(ram["Capacity"]) / 1024 / 1024),
						Convert.ToInt32(ram["Speed"])));
				}
				
				// Add system slots
				foreach (ManagementObject slot in new ManagementClass("Win32_SystemSlot").GetInstances()) {
					string type = "";
					if (((UInt16[])slot["ConnectorType"]).Length > 0) {
						switch (((UInt16[])slot["ConnectorType"])[0]) {
							case 43: type="PCI"; break;
							case 44: type="ISA"; break;
							case 73: type="AGP"; break;
							case 80: type="PCI-66MHz"; break;
							case 81: type="AGP-2X"; break;
							case 82: type="AGP-4X"; break;
							case 98: type="PCI-X"; break;
						}
					}
					motherboard.AppendChild(new Slot(
						slot["SlotDesignation"].ToString(), type, "4" == slot["CurrentUsage"].ToString()));
				}

				return motherboard;
			}
			return base.GetMotherBoard();
		}

		private Component GetBIOS() {
			foreach (ManagementObject mo in new ManagementClass("Win32_BIOS").GetInstances())
				return new BIOS(mo["Manufacturer"].ToString(), mo["SMBIOSBIOSVersion"].ToString(), mo["Version"].ToString());
			return NullComponent.Instance;
		}

		public override Collection GetProcessors() {
			Collection processors = base.GetProcessors();
			foreach (ManagementObject mo in new ManagementClass("Win32_Processor").GetInstances()) {
				processors.AppendChild(new Processor(
					mo["Manufacturer"].ToString(), mo["Name"].ToString().Trim(), 
					Convert.ToInt32(mo["MaxClockSpeed"]), mo["ProcessorId"].ToString()));
			}
			return processors;
		}

		public override Component GetKeyboard() {
			foreach (ManagementObject mo in new ManagementClass("Win32_Keyboard").GetInstances())
				return new Keyboard(mo["Description"].ToString());
			return base.GetKeyboard();
		}

		public override Collection GetPointingDevices() {
			Collection devices = base.GetMonitors();
			foreach (ManagementObject mo in new ManagementClass("Win32_PointingDevice").GetInstances()) {
				if ("Error" != mo["Status"].ToString())
					devices.AppendChild(new PointingDevice(mo["Manufacturer"].ToString(), mo["Description"].ToString()));
			}
			return devices;
		}

		public override Collection GetStorageDevices() {
			Collection devices = base.GetStorageDevices();
			foreach (ManagementObject mo in new ManagementClass("Win32_DiskDrive").GetInstances()) {
				Disk disk = new Disk(
					mo["DeviceID"].ToString(), mo["Model"].ToString(), 
					(int)(Convert.ToInt64(mo["Size"].ToString()) / 1024 / 1024), 0);
				
				// Add partitions
				foreach (ManagementObject map in new ManagementClass("Win32_DiskDriveToDiskPartition").GetInstances()) {
					if (mo.Equals(new ManagementObject(map["Antecedent"].ToString()))) {
						ManagementObject partition = new ManagementObject(map["Dependent"].ToString());
						disk.AppendChild(new Partition(
							partition["Name"].ToString(), 
							(int)(Convert.ToInt64(partition["Size"].ToString()) / 1024 / 1024),
							partition["Type"].ToString()));
					}
				}
				
				devices.AppendChild(disk);
			}
			foreach (ManagementObject mo in new ManagementClass("Win32_CDROMDrive").GetInstances())
				devices.AppendChild(new ROMDrive(mo["Name"].ToString()));
			return devices;
		}

		public override Collection GetNetworkDevices() {
			Collection devices = base.GetNetworkDevices();
			foreach (ManagementObject mo in new ManagementClass("Win32_NetworkAdapter").GetInstances()) {
				// NetConnectionStatus is NULL for non-hardware adapters
				if (null != mo["MACAddress"]/* && null != mo["NetConnectionStatus"]*/) {
					// Ethernet and Wireless adapters
					int types = 0x00 | 0x05 | 0x09;
					int typeid = null != mo["AdapterTypeID"] ? Convert.ToInt32(mo["AdapterTypeID"]) : 0;
					
					if ((typeid | types) == types) {
						NetworkDevice nic = new NetworkDevice(mo["Description"].ToString(), mo["MACAddress"].ToString());

						// Add configured interfaces
						foreach (ManagementObject setting in new ManagementClass("Win32_NetworkAdapterSetting").GetInstances()) {
							if (mo.Equals(new ManagementObject(setting["Element"].ToString()))) {
								ManagementObject config = new ManagementObject(setting["Setting"].ToString());
								if (null != config["IPAddress"] && null != config["IPSubnet"]) {
									String[] ips = (String[])config["IPAddress"];
									String[] netmasks = (String[])config["IPSubnet"];
									for (int i=0; i<ips.Length && i<netmasks.Length; i++) {
										nic.AppendChild(new NetworkInterface(ips[i], netmasks[i]));
									}
								}
							}
						}
						
						devices.AppendChild(nic);
					}
				}
			}
			return devices;
		}

		public override Collection GetVideoCards() {
			Collection videocards = base.GetVideoCards();
			foreach (ManagementObject mo in new ManagementClass("Win32_VideoController").GetInstances()) {
				try {
					videocards.AppendChild(new VideoCard(mo["Description"].ToString(), 
						(int)Convert.ToInt32(mo["AdapterRAM"]) / 1024 / 1024,
						Convert.ToInt32(mo["CurrentHorizontalResolution"]),
						Convert.ToInt32(mo["CurrentVerticalResolution"]),
						Convert.ToInt32(mo["CurrentBitsPerPixel"]),
						Convert.ToInt32(mo["CurrentRefreshRate"])));
				}
				catch (NullReferenceException) { }
			}
			return videocards;
		}

		public override Collection GetSoundCards() {
			Collection soundcards = base.GetSoundCards();
			foreach (ManagementObject mo in new ManagementClass("Win32_SoundDevice").GetInstances())
				soundcards.AppendChild(new SoundCard(mo["Description"].ToString()));
			return soundcards;
		}

		public override Collection GetMonitors() {
			Collection monitors = base.GetMonitors();
			foreach (ManagementObject mo in new ManagementClass("Win32_DesktopMonitor").GetInstances()) {
				try {
					monitors.AppendChild(new Monitor(
						mo["MonitorManufacturer"].ToString(), mo["MonitorType"].ToString(), Math.Sqrt(
						Math.Pow(Convert.ToInt32(mo["ScreenHeight"]) / 100, 2) +
						Math.Pow(Convert.ToInt32(mo["ScreenWidth"]) / 100, 2))));
				}
				catch (NullReferenceException) { }
			}
			return monitors;
		}

		public override Collection GetOperatingSystem() {
			foreach (ManagementObject mo in new ManagementClass("Win32_OperatingSystem").GetInstances()) {
				Collection os = new OperatingSystem(
					Identifier, mo["Manufacturer"].ToString(),
					mo["Name"].ToString().Substring(0, mo["Name"].ToString().IndexOf('|')), 
					mo["Version"].ToString(), mo["CSName"].ToString(),
					mo["SerialNumber"].ToString(), mo["WindowsDirectory"].ToString());
				
				// Add printers
				try {
					foreach (ManagementObject printer in new ManagementClass("Win32_Printer").GetInstances()) {
						try {
							os.AppendChild(new Printer(
								printer["Name"].ToString(),
								printer["DriverName"].ToString(),
								printer["PortName"].ToString()));
						}
						catch (NullReferenceException) { }
					}
				}
				catch (ManagementException) { }

				return os;
			}
			return base.GetOperatingSystem();
		}
	}
}
