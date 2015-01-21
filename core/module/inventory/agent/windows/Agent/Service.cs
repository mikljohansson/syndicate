using System;
using System.Xml;
using System.Threading;
using System.Collections;
using System.Diagnostics;
using System.ServiceProcess;
using System.Configuration.Install;
using System.Runtime.Remoting;
using System.Runtime.Remoting.Channels;
using System.Runtime.Remoting.Channels.Http;

/**
 * Inventory service
 * 
 * Responsible for running the agent when the various hooks are 
 * pulled. 
 *
 * @access		public
 * @package		synd.core.module
 */
namespace Inventory {
	public class Service : ServiceBase {
		public static Service Instance = null;
		public const string Name = "Inventory Agent";
		private System.ComponentModel.Container components = null;

		private XmlDocument _config = null;
		private Thread _worker = null;
		private Agent _agent = null;
		private AutoResetEvent _lock = null;
		private Timer _timer = null;
		private IChannel Channel = null;

		/**
		 * Contructor is run once on computer/serviceprocess startup
		 */
		public Service() {
			Instance = this;
			ServiceName = Name;
		}

		/**
		 * The main entry point for the process
		 * 
		 * This method is run when the program is executed from the 
		 * commandline or when the serviceprocess starts
		 */
		static void Main(string[] args) {
			if (_hasArgument(args, "install")) {
				_install();
				if (_hasArgument(args, "start"))
					_start();
			}
			else if (_hasArgument(args, "uninstall")) 
				_uninstall();
			else if (_hasArgument(args, "start"))
				_start();
			else if (_hasArgument(args, "stop"))
				_stop();
			else {
				ServiceBase[] ServicesToRun = new ServiceBase[] { new Service() };
				ServiceBase.Run(ServicesToRun);
			}
		}

		static private bool _hasArgument(string[] args, string arg) {
			for (int i=0; i<args.Length; i++) {
				if ("/" == args[i].Substring(0,1) && args[i].Substring(1).ToLower() == arg.ToLower() ||
					"--" == args[i].Substring(0,2) && args[i].Substring(2).ToLower() == arg.ToLower())
					return true;
			}
			return false;
		}

		protected override void Dispose(bool disposing) {
			if (disposing && null != components)
				components.Dispose();
			base.Dispose(disposing);
		}

		protected void LoadConfig() {
			string cwd = System.Reflection.Assembly.GetExecutingAssembly().Location;
			XmlTextReader reader = new XmlTextReader(
				cwd.Substring(0, cwd.LastIndexOf("\\")) + "\\Inventory.xml");
			reader.WhitespaceHandling = WhitespaceHandling.None;
			_config = new XmlDocument();
			_config.Load(reader);
		}
		
		protected ITransport GetTransport() {
			XmlNode agent = _config.SelectSingleNode("/Inventory/Agent");
			XmlNode method = agent.Attributes.GetNamedItem("Transport");
			
			switch (method.Value.ToUpper()) {
				case "SOAP":
					XmlNode location = agent.Attributes.GetNamedItem("Location");
					if (null == location)
						throw new ArgumentException("Attribute 'Location' missing from Agent element in config");

					IInventoryModule inventory = (IInventoryModule)Activator.GetObject(typeof(IInventoryModule), location.Value);
					return new SoapTransport(inventory, new XmlFormatter());

				case "MAIL":
					XmlNode server = agent.Attributes.GetNamedItem("Server");
					XmlNode address = agent.Attributes.GetNamedItem("Address");
					if (null == server)
						throw new ArgumentException("Attribute 'Server' missing from Agent element in config");
					if (null == address)
						throw new ArgumentException("Attribute 'Address' missing from Agent element in config");
					
					return new MailTransport(server.Value, address.Value, new XmlFormatter());
			}
			
			throw new ArgumentException("Invalid transport method '" + method.Value + "'");
		}

		/**
		 * Service startup hook
		 */
		protected override void OnStart(string[] args) {
			try {
				// Load Inventory.xml from current directory
				LoadConfig();

				// Attempt to start the remote control listener
				string uri = InitializeChannel();

				// Set to signalled (true) to run agent on startup
				_lock = new AutoResetEvent(true);
				_agent = new Agent(GetTransport(), _config.SelectSingleNode("/Inventory/Agent"), uri);

				_worker = new Thread(new ThreadStart(WorkerThread));
				_worker.Start();

				// Setup a timer to run every 24h
				_timer = new Timer(new TimerCallback(this._callback_timer), new object(),
					this.AgentInterval, this.AgentInterval);
			}
			catch (Exception e) {
				Log(e.ToString(), EventLogEntryType.Warning);
			}
		}

		/**
		 * Service stop hook
		 */
		protected override void OnStop() {
			// Abort worker thread
			_worker.Abort();
			
			// Unregister remoting channels
			if (null != Channel) {
				try {
					ChannelServices.UnregisterChannel(Channel);
				}
				catch (RemotingException) {
				}
				finally {
					Channel = null;
				}
			}

			// Wait for worker to exit
			_worker.Join();
		}

		private TimeSpan AgentInterval {
			get {
				try {
					XmlNode timer = _config.SelectSingleNode("/Inventory/Agent/Timer");
					int hours = Convert.ToInt32(timer.Attributes.GetNamedItem("Interval").Value);
					return new TimeSpan(hours, 0, 0);
				}
				catch (Exception) {
					return new TimeSpan(24, 0, 0);
				}
			}
		}
		
		/**
		 * Attempt to start the RPC service
		 * 
		 * If a port has been configured in inventory.xml the remote
		 * service will start allowing for remote control of the agent,
		 * otherwise no listening port will be opened and the regular
		 * outbound HTTP channel will be registered.
		 * 
		 * @return  string  Return the URI of the remote service or null
		 */
		protected string InitializeChannel() {
			string uri = null;
			try {
				int port = 0;
				XmlNode remote = _config.SelectSingleNode("/Inventory/RemoteService");
				if (null == remote || 0 == (port = Convert.ToInt32(remote.Attributes.GetNamedItem("Port").Value)))
					throw new Exception("No port specified for remote service.");

				// Register the listening server channel
				HttpServerChannel server = new HttpServerChannel(port);
				ChannelServices.RegisterChannel(server, true);

				// Register the remote control interface
				RemotingConfiguration.RegisterWellKnownServiceType(
					typeof(RemoteService), "agent", WellKnownObjectMode.SingleCall);

				Channel = server;
				uri = server.GetChannelUri() + "/agent";
			}
			catch (Exception e) {
				Log(e.ToString(), EventLogEntryType.Information);
				
				// Attempt to register a normal client channel
				try {
					Channel = new HttpClientChannel();
					ChannelServices.RegisterChannel(Channel, true);
				}
				catch (RemotingException) { }
			}
			return uri;
		}

		/**
		 * Timer callback, run every XX hours
		 */
		private void _callback_timer(object state) {
			RunAgent();
		}

		/**
		 * Launch the agent
		 */
		public bool RunAgent() {
			// Check if worker is running
			if (!Monitor.TryEnter(_worker))
				return false;
				
			// Release lock and signal worker
			Monitor.Exit(_worker);
			return _lock.Set();
		}

		/**
		 * Agent worker thread
		 */
		private void WorkerThread() {
			try {
				for (;;) {
					// Wait for signal to run agent
					_lock.WaitOne();
					lock (_worker) {
						_agent.Run();
					}
				}
			}
			catch (ThreadAbortException) {
				// Exit nicely on abort requests
			}
			catch (Exception e) {
				Log(e.ToString(), EventLogEntryType.Warning);
			}
		}

		/**
		 * Attempts to start the service
		 */
		private static void _start() {
			try {
				ServiceController controller  = new ServiceController(Name);
				if (ServiceControllerStatus.Stopped == controller.Status)
					controller.Start();
			}
			catch (Exception e) {
				Log(e.ToString(), EventLogEntryType.Warning);
			}
		}

		/**
		 * Attempts to stop the service
		 */
		private static void _stop() {
			try {
				ServiceController controller  = new ServiceController(Name);
				if (ServiceControllerStatus.Stopped != controller.Status &&
					ServiceControllerStatus.StopPending != controller.Status)
					controller.Stop();
			}
			catch (Exception e) {
				Log(e.ToString(), EventLogEntryType.Warning);
			}
		}

		/**
		 * Attempts to install the service
		 */
		private static void _install() {
			try {
				TransactedInstaller transaction = new TransactedInstaller();
				transaction.Installers.Add(new InventoryInstaller(Name));

				String path = String.Format("/assemblypath={0}",
					System.Reflection.Assembly.GetExecutingAssembly().Location);
				String[] cmdline = {path};

				transaction.Context = new InstallContext("", cmdline);
				transaction.Install(new Hashtable());

				Log("Inventory service installed");
			}
			catch (Exception e) {
				Log("Could not install service: "+e.ToString(), EventLogEntryType.Warning);
			}
		}

		/**
		 * Attempts to uninstall the service
		 */
		private static void _uninstall() {
			try {
				TransactedInstaller transaction = new TransactedInstaller();
				transaction.Installers.Add(new InventoryInstaller(Name));

				String path = String.Format("/assemblypath={0}",
					System.Reflection.Assembly.GetExecutingAssembly().Location);
				String[] cmdline = {path};

				InstallContext context = new InstallContext("", cmdline);
				transaction.Context = context;
				transaction.Uninstall(null);

				Log("Inventory service uninstalled");
			}
			catch (Exception e) {
				Log("Could not uninstall service: "+e.ToString(), EventLogEntryType.Warning);
			}
		}

		/**
		 * Writes an Information type message to the syslog
		 */
		public static void Log(string message) {
			Log(message, EventLogEntryType.Information);
		}

		/**
		 * Writes a message to the syslog
		 */
		public static void Log(string message, EventLogEntryType type) {
			const string source = Name;
			if (!EventLog.SourceExists(source))
				EventLog.CreateEventSource(source, "Application");
			EventLog.WriteEntry(source, message, type);
		}
	}
}
