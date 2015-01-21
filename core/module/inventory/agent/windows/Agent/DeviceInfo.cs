using System;
using System.Reflection;
using System.Collections;

namespace Inventory.DeviceInfo {
	public interface IComponentVisitor {
		void Visit(object device);
		void Visit(object device, ArrayList children); 
	}

	public abstract class ComponentVisitor : IComponentVisitor {
		private MethodInfo _lastMethod = null;
		private object _lastDevice = null;

		public void Visit(object device) {
			MethodInfo method = GetType().GetMethod("Visit", 
				BindingFlags.ExactBinding | BindingFlags.Public | BindingFlags.Instance,
				Type.DefaultBinder, new Type[] {device.GetType()}, new ParameterModifier[0]);
			if (null != method && (_lastMethod != method || _lastDevice != device)) {
				method.Invoke(this, new object[] {device});
				_lastMethod = method;
				_lastDevice = device;
			}
		}

		public void Visit(object device, ArrayList children) {
			MethodInfo method = this.GetType().GetMethod("Visit", 
				BindingFlags.ExactBinding | BindingFlags.Public | BindingFlags.Instance,
				Type.DefaultBinder, new Type[] {device.GetType(), children.GetType()}, new ParameterModifier[0]);
			if (null != method && (_lastMethod != method || _lastDevice != device)) {
				method.Invoke(this, new object[] {device, children});
				_lastMethod = method;
				_lastDevice = device;
			}
		}

		public void Visit(Collection device, ArrayList children) {
			for (IEnumerator i = children.GetEnumerator(); i.MoveNext(); )
				((Component)i.Current).AcceptVisitor(this);
		}
	}

	public abstract class Component {
		public Component Clone() {
			return (Component)this.MemberwiseClone();
		}

		public virtual void AcceptVisitor(IComponentVisitor visitor) {
			visitor.Visit(this);
		}
	}

	public class NullComponent : Component {
		public static Component Instance = new NullComponent();
		
		private NullComponent() {}

		public new Component Clone() {
			return this;
		}

		public override void AcceptVisitor(IComponentVisitor visitor) {}
	}

	public class Collection : Component {
		protected ArrayList _children = new ArrayList();
		
		public virtual void AppendChild(Component child) {
			_children.Add(child);
		}

		public virtual void AppendChild(NullComponent child) {}

		public override void AcceptVisitor(IComponentVisitor visitor) {
			visitor.Visit(this, this._children);
		}
	}

	public abstract class Factory {
		private Factory _next;

		public Factory() {
			_next = null;
		}

		public Factory(Factory next) {
			_next = next;
		}

		public virtual Component GetMotherBoard() {
			return null != _next ? _next.GetMotherBoard() : NullComponent.Instance;
		}

		public virtual Collection GetProcessors() {
			return null != _next ? _next.GetProcessors() : new Collection();
		}

		public virtual Component GetKeyboard() {
			return null != _next ? _next.GetKeyboard() : NullComponent.Instance;
		}

		public virtual Collection GetPointingDevices() {
			return null != _next ? _next.GetPointingDevices() : new Collection();
		}

		public virtual Collection GetStorageDevices() {
			return null != _next ? _next.GetStorageDevices() : new Collection();
		}

		public virtual Collection GetNetworkDevices() {
			return null != _next ? _next.GetNetworkDevices() : new Collection();
		}

		public virtual Collection GetVideoCards() {
			return null != _next ? _next.GetVideoCards() : new Collection();
		}

		public virtual Collection GetSoundCards() {
			return null != _next ? _next.GetSoundCards() : new Collection();
		}

		public virtual Collection GetMonitors() {
			return null != _next ? _next.GetMonitors() : new Collection();
		}

		public virtual Collection GetOperatingSystem() {
			return null != _next ? _next.GetOperatingSystem() : new Collection();
		}
	}
	
	public class Device : Collection {
		public Device() {}
	}
	
	public class Listener : Component {
		public string Uri;
		public string Version;
		public string Action;
		
		public Listener(string uri, string version, string action) {
			Uri = uri;
			Version = version;
			Action = action;
		}
	}

	public class MotherBoard : Collection {
		public string Vendor;
		public string Version;
		public string Release;
		public string Serial;
		
		public MotherBoard(string vendor, string version, string release, string serial) {
			Vendor = vendor;
			Version = version;
			Release = release;
			Serial = serial;
		}
	}

	public class BIOS : Component {
		public string Vendor;
		public string Version;
		public string Release;
		
		public BIOS(string vendor, string version, string release) {
			Vendor = vendor;
			Version = version;
			Release = release;
		}
	}

	public class RAM : Component {
		public int Size;
		public string Type;
		public string Bank;
		public string Slot;
		public int Frequency;

		public RAM(string slot, string type, string bank, int size, int frequency) {
			Slot = slot;
			Type = type;
			Bank = bank;
			Size = size;
			Frequency = frequency;
		}
	}

	public class Port : Component {
		public string Name;
		public string Type;
		public string Connector;
		
		public Port(string name, string type, string connector) {
			Name = name;
			Type = type;
			Connector = connector;
		}
	}

	public class Slot : Component {
		public string Name;
		public string Type;
		public bool Occupied;
		
		public Slot(string name, string type, bool occupied) {
			Name = name;
			Type = type;
			Occupied = occupied;
		}
	}

	public class Processor : Component {
		public string Vendor;
		public string Version;
		public int Frequency;
		public string Serial;
		
		public Processor(string vendor, string version, int frequency, string serial) {
			Vendor = vendor;
			Version = version;
			Frequency = frequency;
			Serial = serial;
		}
	}

	public class Keyboard : Component {
		public string Version;

		public Keyboard(string version) {
			Version = version;
		}
	}

	public class PointingDevice : Component {
		public string Vendor;
		public string Version;

		public PointingDevice(string vendor, string version) {
			Vendor = vendor;
			Version = version;
		}
	}

	public class Disk : Collection {
		public string Name;
		public string Version;
		public int Size;
		public int Cache;

		public Disk(string name, string version, int size, int cache) {
			Name = name;
			Version = version;
			Size = size;
			Cache = cache;
		}
	}

	public class Partition : Component {
		public string Name;
		public int Size;
		public string Type;

		public Partition(string name, int size, string type) {
			Name = name;
			Size = size;
			Type = type;
		}
	}

	public class ROMDrive : Component {
		public string Version;

		public ROMDrive(string version) {
			Version = version;
		}
	}

	public class NetworkDevice : Collection {
		public string Version;
		public string MAC;
		
		public NetworkDevice(string version, string mac) {
			Version = version;
			MAC = mac;
		}
	}

	public class NetworkInterface : Component {
		public string IP;
		public string Netmask;
		
		public NetworkInterface(string ip, string netmask) {
			IP = ip;
			Netmask = netmask;
		}
	}
	public class VideoCard : Component {
		public string Version;
		public int RAM;
		public int HorizontalResolution;
		public int VerticalResolution;
		public int BitsPerPixel;
		public int Frequency;
		
		public VideoCard(string version, int ram, int hres, int vres, int bits, int frequency) {
			Version = version;
			RAM = ram;
			HorizontalResolution = hres;
			VerticalResolution = vres;
			BitsPerPixel = bits;
			Frequency = frequency;
		}
	}

	public class SoundCard : Component {
		public string Version;
		
		public SoundCard(string version) {
			Version = version;
		}
	}

	public class Monitor : Component {
		public string Vendor;
		public string Version;
		public double Size;
		
		public Monitor(string vendor, string version, double size) {
			Vendor = vendor;
			Version = version;
			Size = size;
		}
	}

	public class OperatingSystem : Collection {
		public string Name;
        public string Vendor;
		public string Version;
		public string Release;
		public string MachineName;
		public string SerialNumber;
		public string Path;

		public OperatingSystem(string name, string vendor, string version, string release, string machinename, string serial, string path) {
			Name = name;
			Vendor = vendor;
			Version = version;
			Release = release;
			MachineName = machinename;
			SerialNumber = serial;
			Path = path;
		}
	}

	public class Printer : Component {
		public string Name;
		public string Driver;
		public string Port;

		public Printer(string name, string driver, string port) {
			Name = name;
			Driver = driver;
			Port = port;
		}
	}

	public class Product : Component {
		public string Vendor;
		public string Name;
		public string Version;
		public string InstallDate;
		public bool State;

		public Product(string vendor, string name, string version, string installdate, bool state) {
			Vendor = vendor;
			Name = name;
			Version = version;
			InstallDate = installdate;
			State = state;
		}
	}

	public class Image : Component {
		public string Version;

		public Image(string version) {
			Version = version;
		}
	}
}
