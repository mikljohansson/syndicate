using System;

namespace Inventory {
	public class RemoteService : MarshalByRefObject {
		public const string SoapAction = "http://schemas.microsoft.com/clr/nsassem/Inventory.RemoteService/Inventory#{method}";
		public const string ApiVersion = "20051205";
		
		public RemoteService() {
		}

		public bool run() {
			return Service.Instance.RunAgent();
		}
	}
}
