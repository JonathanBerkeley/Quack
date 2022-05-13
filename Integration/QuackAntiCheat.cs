using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;


public class QuackAntiCheat : MonoBehaviour
{

    [DllImport("kernel32.dll")]
    public static extern System.IntPtr LoadLibrary(string library);

    void Start()
    {
        System.IntPtr anti_cheat = LoadLibrary(@"Quack-internal");
        if (anti_cheat == System.IntPtr.Zero)
        {
            Debug.Log($"Could not load library: {Marshal.GetLastWin32Error()}");
            Application.Quit();
        }
    }
}
