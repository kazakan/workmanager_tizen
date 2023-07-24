import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

import 'workmanager_tizen_platform_interface.dart';

/// An implementation of [WorkmanagerTizenPlatform] that uses method channels.
class MethodChannelWorkmanagerTizen extends WorkmanagerTizenPlatform {
  /// The method channel used to interact with the native platform.
  @visibleForTesting
  final methodChannel = const MethodChannel('workmanager_tizen');

  @override
  Future<String?> getPlatformVersion() async {
    final version =
        await methodChannel.invokeMethod<String>('getPlatformVersion');
    return version;
  }
}
