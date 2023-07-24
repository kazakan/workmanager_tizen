import 'package:plugin_platform_interface/plugin_platform_interface.dart';

import 'workmanager_tizen_method_channel.dart';

abstract class WorkmanagerTizenPlatform extends PlatformInterface {
  /// Constructs a WorkmanagerTizenPlatform.
  WorkmanagerTizenPlatform() : super(token: _token);

  static final Object _token = Object();

  static WorkmanagerTizenPlatform _instance = MethodChannelWorkmanagerTizen();

  /// The default instance of [WorkmanagerTizenPlatform] to use.
  ///
  /// Defaults to [MethodChannelWorkmanagerTizen].
  static WorkmanagerTizenPlatform get instance => _instance;

  /// Platform-specific implementations should set this with their own
  /// platform-specific class that extends [WorkmanagerTizenPlatform] when
  /// they register themselves.
  static set instance(WorkmanagerTizenPlatform instance) {
    PlatformInterface.verifyToken(instance, _token);
    _instance = instance;
  }

  Future<String?> getPlatformVersion() {
    throw UnimplementedError('platformVersion() has not been implemented.');
  }
}
