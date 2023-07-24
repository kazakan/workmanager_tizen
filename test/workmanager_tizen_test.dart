import 'package:flutter_test/flutter_test.dart';
import 'package:workmanager_tizen/workmanager_tizen.dart';
import 'package:workmanager_tizen/workmanager_tizen_platform_interface.dart';
import 'package:workmanager_tizen/workmanager_tizen_method_channel.dart';
import 'package:plugin_platform_interface/plugin_platform_interface.dart';

class MockWorkmanagerTizenPlatform
    with MockPlatformInterfaceMixin
    implements WorkmanagerTizenPlatform {

  @override
  Future<String?> getPlatformVersion() => Future.value('42');
}

void main() {
  final WorkmanagerTizenPlatform initialPlatform = WorkmanagerTizenPlatform.instance;

  test('$MethodChannelWorkmanagerTizen is the default instance', () {
    expect(initialPlatform, isInstanceOf<MethodChannelWorkmanagerTizen>());
  });

  test('getPlatformVersion', () async {
    WorkmanagerTizen workmanagerTizenPlugin = WorkmanagerTizen();
    MockWorkmanagerTizenPlatform fakePlatform = MockWorkmanagerTizenPlatform();
    WorkmanagerTizenPlatform.instance = fakePlatform;

    expect(await workmanagerTizenPlugin.getPlatformVersion(), '42');
  });
}
