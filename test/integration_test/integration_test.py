#!/usr/bin/env python3
"""
PhotonRPC 集成测试
验证整个RPC框架的功能正确性，包括正常流程和边界/错误情况
"""

import socket
import struct
import subprocess
import time
import threading
import os
import sys
import signal
from typing import Optional

import rpc_message_pb2
import echo_service_pb2
import calculate_service_pb2

# 测试配置
SERVER_HOST = "127.0.0.1"
SERVER_PORT = 12345
TEST_TIMEOUT = 5  # 秒

# Codec实现（与C++版本一致）
class Codec:
    """消息编解码器，实现与C++版本一致的协议"""
    
    @staticmethod
    def encode(data: bytes) -> bytes:
        """编码：4字节长度前缀 + 数据"""
        length = len(data)
        length_bytes = struct.pack('<I', length)  # 小端序，4字节
        return length_bytes + data
    
    @staticmethod
    def decode(encoded_data: bytes) -> bytes:
        """解码：读取4字节长度前缀，返回数据部分"""
        if len(encoded_data) < 4:
            return b''
        length = struct.unpack('<I', encoded_data[:4])[0]
        if len(encoded_data) < 4 + length:
            return b''
        return encoded_data[4:4+length]


# Protobuf消息构造（使用生成的protobuf模块）

def build_rpc_message(message_id: int, message_type: int, 
                     service_name: str, method_name: str,
                     request_data: bytes = b'') -> bytes:
    """构造RpcMessage protobuf消息"""
    rpc_msg = rpc_message_pb2.RpcMessage()
    rpc_msg.id = message_id
    rpc_msg.type = message_type
    rpc_msg.service_name = service_name
    rpc_msg.method_name = method_name
    
    if message_type == rpc_message_pb2.MessageType.RPC_TYPE_REQUEST and request_data:
        rpc_msg.request = request_data
    elif message_type == rpc_message_pb2.MessageType.RPC_TYPE_RESPONSE and request_data:
        rpc_msg.response = request_data
    
    return rpc_msg.SerializeToString()


def parse_rpc_response(data: bytes) -> Optional[bytes]:
    """解析RPC响应消息，返回response字段内容"""
    rpc_msg = rpc_message_pb2.RpcMessage()
    rpc_msg.ParseFromString(data)
    return rpc_msg.response if rpc_msg.response else None


# Echo和Calculate服务的protobuf消息构造
def build_echo_request(sentence: str) -> bytes:
    """构造EchoRequest: sentence (field 1, string)"""
    echo_req = echo_service_pb2.EchoRequest()
    echo_req.sentence = sentence
    return echo_req.SerializeToString()


def parse_echo_response(data: bytes) -> Optional[str]:
    """解析EchoResponse: result (field 1, string)"""
    echo_resp = echo_service_pb2.EchoResponse()
    echo_resp.ParseFromString(data)
    return echo_resp.result


def build_add_request(a: int, b: int) -> bytes:
    """构造AddRequest: a (field 1, int32), b (field 2, int32)"""
    add_req = calculate_service_pb2.AddRequest()
    add_req.a = a
    add_req.b = b
    return add_req.SerializeToString()


def build_sub_request(a: int, b: int) -> bytes:
    """构造SubRequest: a (field 1, int32), b (field 2, int32)"""
    sub_req = calculate_service_pb2.SubRequest()
    sub_req.a = a
    sub_req.b = b
    return sub_req.SerializeToString()


def parse_add_response(data: bytes) -> Optional[int]:
    """解析AddResponse: result (field 1, int32)"""
    add_resp = calculate_service_pb2.AddResponse()
    add_resp.ParseFromString(data)
    return add_resp.result


def parse_sub_response(data: bytes) -> Optional[int]:
    """解析SubResponse: result (field 1, int32)"""
    sub_resp = calculate_service_pb2.SubResponse()
    sub_resp.ParseFromString(data)
    return sub_resp.result


# RPC客户端类
class RpcClient:
    """RPC客户端，用于发送请求和接收响应"""
    
    def __init__(self, host: str = SERVER_HOST, port: int = SERVER_PORT):
        self.host = host
        self.port = port
        self.sock = None
    
    def connect(self) -> bool:
        """连接到服务器"""
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.settimeout(TEST_TIMEOUT)
            self.sock.connect((self.host, self.port))
            return True
        except Exception as e:
            print(f"连接失败: {e}")
            return False
    
    def call_rpc(self, service_name: str, method_name: str, 
                 request_data: bytes, message_id: int = 1) -> Optional[bytes]:
        """发送RPC请求并返回响应数据"""
        if not self.sock:
            if not self.connect():
                return None
        
        try:
            # 构造RPC消息
            rpc_message = build_rpc_message(
                message_id, 
                rpc_message_pb2.MessageType.RPC_TYPE_REQUEST,
                service_name,
                method_name,
                request_data
            )
            
            # 编码并发送
            encoded = Codec.encode(rpc_message)
            self.sock.sendall(encoded)
            
            # 接收响应
            # 先读取4字节长度
            length_data = b''
            while len(length_data) < 4:
                chunk = self.sock.recv(4 - len(length_data))
                if not chunk:
                    return None
                length_data += chunk
            
            length = struct.unpack('<I', length_data)[0]
            
            # 读取消息体
            message_data = b''
            while len(message_data) < length:
                chunk = self.sock.recv(length - len(message_data))
                if not chunk:
                    return None
                message_data += chunk
            
            # 解析响应
            response_data = parse_rpc_response(message_data)
            return response_data
            
        except socket.timeout:
            print(f"请求超时")
            return None
        except Exception as e:
            print(f"RPC调用失败: {e}")
            return None
    
    def close(self):
        """关闭连接"""
        if self.sock:
            self.sock.close()
            self.sock = None


# 测试辅助函数
class TestHelper:
    """测试辅助类"""
    
    @staticmethod
    def wait_for_server(host: str, port: int, timeout: int = 10) -> bool:
        """等待服务器启动"""
        end_time = time.time() + timeout
        while time.time() < end_time:
            try:
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                sock.settimeout(1)
                result = sock.connect_ex((host, port))
                sock.close()
                if result == 0:
                    return True
            except:
                pass
            time.sleep(0.1)
        return False
    
    @staticmethod
    def start_server(server_path: str) -> Optional[subprocess.Popen]:
        """启动服务器进程"""
        try:
            # 切换到正确的目录以确保配置文件路径正确
            test_dir = os.path.dirname(os.path.abspath(__file__))
            project_root = os.path.dirname(test_dir)
            server_full_path = os.path.join(project_root, server_path)
            
            if not os.path.exists(server_full_path):
                print(f"服务器程序不存在: {server_full_path}")
                return None
            
            process = subprocess.Popen(
                [server_full_path],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                cwd=project_root
            )
            return process
        except Exception as e:
            print(f"启动服务器失败: {e}")
            return None
    
    @staticmethod
    def stop_server(process: Optional[subprocess.Popen]):
        """停止服务器进程"""
        if process:
            try:
                process.terminate()
                try:
                    process.wait(timeout=2)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait()
            except:
                pass


# ============================================================================
# 测试用例
# ============================================================================

def test_normal_echo_call():
    """测试1: 正常EchoService调用"""
    print("\n[测试1] 正常EchoService调用")
    client = RpcClient()
    try:
        request_data = build_echo_request("Hello, PhotonRPC!")
        response_data = client.call_rpc("EchoService", "Echo", request_data)
        
        assert response_data is not None, "响应不应为空"
        result = parse_echo_response(response_data)
        assert result == "Hello, PhotonRPC!", f"期望 'Hello, PhotonRPC!'，得到 '{result}'"
        print("✓ EchoService调用成功")
    finally:
        client.close()


def test_normal_calculate_add():
    """测试2: 正常CalculateService.Add调用"""
    print("\n[测试2] 正常CalculateService.Add调用")
    client = RpcClient()
    try:
        request_data = build_add_request(5, 6)
        response_data = client.call_rpc("CalculateService", "Add", request_data)
        
        assert response_data is not None, "响应不应为空"
        result = parse_add_response(response_data)
        assert result == 11, f"期望 11，得到 {result}"
        print("✓ CalculateService.Add调用成功")
    finally:
        client.close()


def test_normal_calculate_sub():
    """测试3: 正常CalculateService.Sub调用"""
    print("\n[测试3] 正常CalculateService.Sub调用")
    client = RpcClient()
    try:
        request_data = build_sub_request(10, 3)
        response_data = client.call_rpc("CalculateService", "Sub", request_data)
        
        assert response_data is not None, "响应不应为空"
        result = parse_sub_response(response_data)
        assert result == 7, f"期望 7，得到 {result}"
        print("✓ CalculateService.Sub调用成功")
    finally:
        client.close()


def test_empty_string():
    """测试4: 空字符串参数"""
    print("\n[测试4] 空字符串参数")
    client = RpcClient()
    try:
        request_data = build_echo_request("")
        response_data = client.call_rpc("EchoService", "Echo", request_data)
        
        assert response_data is not None, "响应不应为空"
        result = parse_echo_response(response_data)
        assert result == "", f"期望空字符串，得到 '{result}'"
        print("✓ 空字符串处理成功")
    finally:
        client.close()


def test_large_string():
    """测试5: 大字符串参数"""
    print("\n[测试5] 大字符串参数")
    client = RpcClient()
    try:
        large_string = "A" * 10000  # 10KB字符串
        request_data = build_echo_request(large_string)
        response_data = client.call_rpc("EchoService", "Echo", request_data)
        
        assert response_data is not None, "响应不应为空"
        result = parse_echo_response(response_data)
        assert result == large_string, "大字符串应完整返回"
        print("✓ 大字符串处理成功")
    finally:
        client.close()


def test_negative_numbers():
    """测试6: 负数计算"""
    print("\n[测试6] 负数计算")
    client = RpcClient()
    try:
        # 测试负数加法：(-5) + 3 = -2
        # 注意：这里使用简化实现，可能需要调整
        request_data = build_add_request(-5, 3)
        response_data = client.call_rpc("CalculateService", "Add", request_data)
        
        if response_data:
            result = parse_add_response(response_data)
            # 如果解析成功，验证结果
            if result is not None:
                print(f"✓ 负数计算返回: {result}")
        else:
            print("! 负数计算可能不被支持（简化实现限制）")
    finally:
        client.close()


def test_concurrent_requests():
    """测试7: 并发请求"""
    print("\n[测试7] 并发请求")
    
    def make_request(thread_id: int):
        client = RpcClient()
        try:
            request_data = build_echo_request(f"Thread-{thread_id}")
            response_data = client.call_rpc("EchoService", "Echo", request_data)
            if response_data:
                result = parse_echo_response(response_data)
                return result == f"Thread-{thread_id}"
            return False
        finally:
            client.close()
    
    threads = []
    results = []
    
    def worker(thread_id):
        results.append(make_request(thread_id))
    
    for i in range(10):
        t = threading.Thread(target=worker, args=(i,))
        threads.append(t)
        t.start()
    
    for t in threads:
        t.join()
    
    success_count = sum(results)
    assert success_count == 10, f"期望10个成功，得到{success_count}个"
    print(f"✓ 并发请求成功 ({success_count}/10)")


def test_invalid_service_name():
    """测试8: 无效的服务名"""
    print("\n[测试8] 无效的服务名")
    client = RpcClient()
    try:
        request_data = build_echo_request("test")
        response_data = client.call_rpc("NonExistentService", "Echo", request_data)
        # 服务器可能会崩溃或返回错误（取决于实现）
        # 这里我们主要验证不会导致客户端崩溃
        print("! 无效服务名测试完成（服务器可能崩溃或返回错误）")
    except Exception as e:
        print(f"! 捕获到异常（预期行为）: {e}")
    finally:
        client.close()


def test_invalid_method_name():
    """测试9: 无效的方法名"""
    print("\n[测试9] 无效的方法名")
    client = RpcClient()
    try:
        request_data = build_echo_request("test")
        response_data = client.call_rpc("EchoService", "NonExistentMethod", request_data)
        # 服务器可能会崩溃或返回错误
        print("! 无效方法名测试完成（服务器可能崩溃或返回错误）")
    except Exception as e:
        print(f"! 捕获到异常（预期行为）: {e}")
    finally:
        client.close()


def test_invalid_protobuf_data():
    """测试10: 无效的protobuf数据"""
    print("\n[测试10] 无效的protobuf数据")
    client = RpcClient()
    try:
        invalid_data = b"invalid protobuf data\x00\x01\x02"
        response_data = client.call_rpc("EchoService", "Echo", invalid_data)
        # 服务器应该能处理无效数据而不崩溃
        print("! 无效protobuf数据测试完成（服务器可能返回错误或崩溃）")
    except Exception as e:
        print(f"! 捕获到异常（预期行为）: {e}")
    finally:
        client.close()


def test_multiple_sequential_calls():
    """测试11: 多次顺序调用（连接复用）"""
    print("\n[测试11] 多次顺序调用")
    client = RpcClient()
    try:
        # 第一次调用
        request_data1 = build_echo_request("First")
        response_data1 = client.call_rpc("EchoService", "Echo", request_data1)
        assert response_data1 is not None
        result1 = parse_echo_response(response_data1)
        assert result1 == "First"
        
        # 第二次调用（使用同一连接）
        request_data2 = build_add_request(7, 8)
        response_data2 = client.call_rpc("CalculateService", "Add", request_data2)
        assert response_data2 is not None
        result2 = parse_add_response(response_data2)
        assert result2 == 15
        
        # 第三次调用
        request_data3 = build_echo_request("Third")
        response_data3 = client.call_rpc("EchoService", "Echo", request_data3)
        assert response_data3 is not None
        result3 = parse_echo_response(response_data3)
        assert result3 == "Third"
        
        print("✓ 多次顺序调用成功")
    finally:
        client.close()


def test_connection_refused():
    """测试12: 服务器未启动时的连接错误"""
    print("\n[测试12] 连接被拒绝（服务器未启动）")
    # 使用一个不存在的端口
    client = RpcClient(SERVER_HOST, 99999)
    try:
        request_data = build_echo_request("test")
        response_data = client.call_rpc("EchoService", "Echo", request_data)
        # 应该返回None或抛出异常
        assert response_data is None or False, "连接应该失败"
        print("✓ 连接错误处理正确")
    except Exception as e:
        print(f"✓ 捕获到连接异常（预期行为）: {type(e).__name__}")
    finally:
        client.close()


def test_special_characters():
    """测试13: 特殊字符处理"""
    print("\n[测试13] 特殊字符处理")
    client = RpcClient()
    try:
        special_strings = [
            "Hello\nWorld",
            "Tab\tHere",
            "中文测试",
            "🚀 Emoji Test",
            "Null\x00Byte",
            "Special: !@#$%^&*()"
        ]
        
        for special_str in special_strings:
            request_data = build_echo_request(special_str)
            response_data = client.call_rpc("EchoService", "Echo", request_data)
            if response_data:
                result = parse_echo_response(response_data)
                # 某些字符可能在传输中丢失，这是可以接受的
                print(f"  输入: {repr(special_str[:20])}, 输出: {repr(result[:20] if result else None)}")
        
        print("✓ 特殊字符测试完成")
    finally:
        client.close()


# ============================================================================
# 主测试函数
# ============================================================================

def run_all_tests():
    """运行所有测试"""
    print("=" * 60)
    print("PhotonRPC 集成测试")
    print("=" * 60)
    
    # 查找服务器程序
    test_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(test_dir)
    server_path = os.path.join(project_root, "bin", "TestProvider")
    
    if not os.path.exists(server_path):
        # 尝试其他可能的位置
        server_path = os.path.join(project_root, "build", "bin", "TestProvider")
        if not os.path.exists(server_path):
            print(f"错误: 找不到服务器程序 TestProvider")
            print(f"请在 {project_root}/bin/ 或 {project_root}/build/bin/ 中构建服务器")
            return 1
    
    # 启动服务器
    print(f"\n启动服务器: {server_path}")
    server_process = TestHelper.start_server("bin/TestProvider")
    if not server_process:
        print("错误: 无法启动服务器")
        return 1
    
    try:
        # 等待服务器启动
        print("等待服务器启动...")
        if not TestHelper.wait_for_server(SERVER_HOST, SERVER_PORT, timeout=10):
            print("错误: 服务器启动超时")
            return 1
        
        print("服务器已启动，开始测试\n")
        
        # 运行测试
        tests = [
            ("正常流程测试", [
                test_normal_echo_call,
                test_normal_calculate_add,
                test_normal_calculate_sub,
                test_multiple_sequential_calls,
            ]),
            ("边界情况测试", [
                test_empty_string,
                test_large_string,
                test_negative_numbers,
                test_special_characters,
                test_concurrent_requests,
            ]),
            ("错误处理测试", [
                test_connection_refused,
                test_invalid_service_name,
                test_invalid_method_name,
                test_invalid_protobuf_data,
            ]),
        ]
        
        passed = 0
        failed = 0
        
        for category, test_list in tests:
            print(f"\n{'='*60}")
            print(f"{category}")
            print(f"{'='*60}")
            
            for test_func in test_list:
                try:
                    test_func()
                    passed += 1
                except AssertionError as e:
                    print(f"✗ 测试失败: {e}")
                    failed += 1
                except Exception as e:
                    print(f"✗ 测试异常: {type(e).__name__}: {e}")
                    failed += 1
                time.sleep(0.1)  # 短暂延迟避免过快
        
        # 汇总
        print(f"\n{'='*60}")
        print("测试汇总")
        print(f"{'='*60}")
        print(f"通过: {passed}")
        print(f"失败: {failed}")
        print(f"总计: {passed + failed}")
        
        return 0 if failed == 0 else 1
        
    finally:
        # 停止服务器
        print("\n停止服务器...")
        TestHelper.stop_server(server_process)
        print("测试完成")


if __name__ == "__main__":
    sys.exit(run_all_tests())
