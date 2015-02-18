Using devices in DGI Modules
============================
All devices are accessed in DGI using the _CDeviceManager_. Refer to the wiki page on the device manager to see how to use it to collect a set of devices. In order to access the new device you have defined, use the string identifier in the `<id>` tag (`ExampleDevice`) in the device manager function calls. The following discussion will discuss the various ways in which a `CDevice` pointer can be used once it has been retrieved from the device manager.

## Reading a Device State
It is possible to read each value specified as a `<state>` in the _device.xml_ specification using the device pointer. This is done through the `CDevice::GetState` function. To access the current value of `Readable` on a device pointer retrieved from the device manager:

```
SignalValue value = device->GetState("Readable");
```

The string passed to the `CDevice::GetState` function must correspond to some value in a `<state>` tag or the code will throw an exception. The function `CDevice::HasState` can be used to check if an exception will be thrown in advance. In general, the following code is safer than the code above:

```
if( device->HasState("Readable") )
  value = device->GetState("Readable");
```

The `CDevice::HasState` function will return false if the state is not recognized (and thus would throw an exception), which will prevent the `CDevice::GetState` function from executing.

## Setting a Device Command
It is possible to set each value specified as a `<command>` in _device.xml_. These commands cannot be read by the DGI, so it is impossible for the current version of DGI to read the value of a command it has issued to some device. To set a command, the following function calls can be used:

```
if( device->HasCommand("Writable") )
  device->SetCommand("Writable", 1.0);
```

The `CDevice::HasCommand` function works the same as its state counter-part. The `CDevice::SetCommand` function in this instance would set the _Writable_ command of the device object to 1. All commands in the DGI have the type _SignalValue_ which is defined to be a float. It is not possible to set a non-numeric command.

# Devices and Inheritance
The device types in _device.xml_ support inheritance in that one device type can inherit all the states and commands of another type. There is no limit on the number of types one device can inherit from, or the depth of the inheritance. Inheritance can be defined using the `<extends>type</extends>` tag.