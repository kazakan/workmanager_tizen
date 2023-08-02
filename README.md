# workmanager_tizen

[![pub package](https://img.shields.io/pub/v/workmanager_tizen.svg)](https://pub.dev/packages/workmanager_tizen)

The Tizen implementation of [`workmanager`](https://pub.dev/packages/workmanager).

## Usage

 This package is not an _endorsed_ implementation of `workmanager`. Therefore, you have to include `workmanager_tizen` alongside `workmanager` as dependencies in your `pubspec.yaml` file.

 ```yaml
dependencies:
  workmanager: ^0.5.1
  workmanager_tizen: ^0.0.1
```

Then you can import `workmanager` in your Dart code:

```dart
import 'package:workmanager/workmanager.dart';
```

For detailed usage, see https://pub.dev/packages/workmanager#usage.

## Required privileges

To use this plugin, you need to declare privileges in `tizen-manifest.xml` of your ui and service application.

``` xml
<privileges>
  <privilege>http://tizen.org/privilege/appmanager.launch</privilege>
</privileges>
```

## Supported devices

- Galaxy Watch series (running Tizen 5.5 or above)

## Limitations

This plugin does not support TV and TV emulations.

The following options are not supported.
- ``
- ``
- ``

This plugin only supports `multi` type of flutter-tizen application.

`callbackDispatcher` should be entry point of service app.

Service app name should be `"your_ui_app_name"+"_service"`
