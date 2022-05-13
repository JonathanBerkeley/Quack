# Identify
Provides an interface to the hardware ID system used in Quack so it can be easily included in games as a .DLL depedency.    

# Usage
### Unity example
```csharp
using System.Runtime.InteropServices;
using System.Text;

public class HardwareID
{
    [DllImport("Identify.dll")]
    static extern void GetHWID(StringBuilder message);
    
    public static string GetHardwareID()
    {
        StringBuilder hwid = new StringBuilder(64);
        GetHWID(hwid);
        return hwid.ToString();
    }
}
```

Example calling code:
```csharp
...
void Start() {
    string hwid = HardwareID.GetHardwareID();
    ...
}
...
```