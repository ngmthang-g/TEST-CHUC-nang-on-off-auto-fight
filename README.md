# Thần Long AutoFight Test

Repository test độc lập cho đúng một mục tiêu: **bật / tắt AutoFight (Đánh quái) bằng action semantic bên trong client**, không giả lập chuột và không click tab `Đánh quái` trên màn hình.

## Nền source

- Nền: source người dùng cung cấp `ThanLongCleanRoute_v1.0.0_FIXED_GitHubActions`.
- Repo nghiên cứu tham khảo duy nhất: `ngmthang-g/clinent-game-than-long-DATA-2222`.
- Repo DATA là **read-only knowledge source** đối với dự án này; không ghi code test vào đó.

## Contract AutoFight đã xác minh từ knowledge base

```text
C_AutoModel.Train = 1
TopIcon:AutoTrainClick()
  -> GUI.FindUI("AutoFight_Main")
  -> StartAutoFight(C_AutoModel.Train)

TopIcon:AutoStopClick()
  -> StartAutoFight(C_AutoModel.None)
```

`Đánh quái` nhìn thấy trong `AutoFight` là tab cấu hình, không phải lệnh bật Train.

## Cách bản test v0.1.0 hoạt động

```text
Controller
 -> chọn PID có GameAssembly.dll
 -> WH_GETMESSAGE bridge kế thừa từ CleanRoute
 -> shared-memory command StartAutoFight / StopAutoFight
 -> IL2CPP metadata resolver
 -> LuaSystemManager.ExecuteFunction(...)
 -> TopIcon.AutoTrainClick / TopIcon.AutoStopClick
 -> client tự chạy semantic AutoFight engine
```

UI test chỉ có:

- Quét client;
- BẬT AUTO FIGHT (ĐÁNH QUÁI);
- TẮT AUTO FIGHT;
- trạng thái và log.

Các control giữ bãi của source nền không còn được đưa ra UI test.

## Trạng thái bằng chứng

- Semantic Lua start/stop: **VERIFIED từ DATA knowledge base**.
- Source adapter v0.1.0: **EXPERIMENTAL**.
- Build/CI: xem GitHub Actions của version hiện tại.
- Runtime AutoFight: **RUNTIME UNTESTED** cho tới khi chạy trên client thật và quan sát hiệu ứng bật/tắt.

Quan trọng: knowledge base chuẩn của dự án lớn yêu cầu mutable action production đi qua `System.Action -> MainThread.Execute`. Bản v0.1.0 cố ý tái sử dụng bridge callback-thread của source nền để kiểm tra hẹp đường Lua semantic trước. Nếu runtime crash/disconnect/no-op, không được kết luận AutoFight contract sai; bước kế tiếp là chuyển action execution sang MainThread bridge đã được thiết kế trong DATA KB.

## Test runtime

1. Mở game, vào nhân vật và đứng ở map đã load xong.
2. Chạy `ThanLongAutoFightTest_v0.1.0.exe` cùng mức quyền với game.
3. Bấm `QUÉT CLIENT`, chọn đúng PID.
4. Bấm `BẬT AUTO FIGHT (ĐÁNH QUÁI)`.
5. Xác nhận trong game: Auto Train thực sự bật và nhân vật bắt đầu hành vi đánh quái theo setting có sẵn.
6. Bấm `TẮT AUTO FIGHT`.
7. Xác nhận Auto Train thực sự dừng.
8. Nếu lỗi, giữ nguyên dòng log bridge để dùng làm evidence cho version sau.

Log `ExecuteFunction signature chưa hỗ trợ` nghĩa là adapter cần resolve **đúng signature hẹp** của `LuaSystemManager.ExecuteFunction`; không phải lý do để broad reverse-engineer lại client.

## Build

Yêu cầu Zig. Trên Windows:

```bat
build.cmd
```

Output:

```text
dist/ThanLongAutoFightTest_v0.1.0.exe
dist/ThanLongAutoFightTestBridge.dll
```

GitHub Actions cũng build và upload hai file này thành artifact.

## AI startup

Trước khi sửa version tiếp theo, đọc theo thứ tự:

1. `AI_PROJECT_KNOWLEDGE_PROTOCOL_V2_OPTIMIZED.md`
2. `AI_CLIENT_ANALYSIS_RULES.txt`
3. `AI_INDEX.md`
4. `PROJECT_KNOWLEDGE.md`
5. `docs/features/AUTO_FIGHT.md`
6. `CHANGELOG.md`
7. source liên quan.
