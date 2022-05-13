# Unity games
For a Unity game to load Quack, the simple script "QuackAntiCheat.cs" is included.    
Attach it to a GameObject in the scene that QuackAntiCheat should be loaded in, e.g a GameObject in multiplayer scene.    

# C++ / other
Simply call ```LoadLibraryA("Quack-internal")``` at the appropriate time.    
Example implementation:    
```cpp
int main() {
    if (!LoadLibraryA("Quack-internal"))
        return -1;

    // Game logic
}
```
