#include "controller_support.inl"

class App {
public:
    bool Create(HINSTANCE instance) {
        INITCOMMONCONTROLSEX controls{sizeof(controls), ICC_STANDARD_CLASSES};
        InitCommonControlsEx(&controls);

        WNDCLASSEXW wc{};
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = WndProc;
        wc.hInstance = instance;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        wc.lpszClassName = L"ThanLongAutoFightTestWindow";
        if (!RegisterClassExW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) return false;

        hwnd_ = CreateWindowExW(0, wc.lpszClassName, kTitle,
                                WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                                CW_USEDEFAULT, CW_USEDEFAULT, 690, 410,
                                nullptr, nullptr, instance, this);
        return hwnd_ != nullptr;
    }

    void Show(int cmd) {
        ShowWindow(hwnd_, cmd);
        UpdateWindow(hwnd_);
    }

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
        App* self = reinterpret_cast<App*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (msg == WM_NCCREATE) {
            auto* create = reinterpret_cast<CREATESTRUCTW*>(lp);
            self = reinterpret_cast<App*>(create->lpCreateParams);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
            self->hwnd_ = hwnd;
        }
        return self ? self->Handle(msg, wp, lp) : DefWindowProcW(hwnd, msg, wp, lp);
    }

    HWND Make(const wchar_t* cls, const wchar_t* text, DWORD style,
              int x, int y, int w, int h, int id) {
        return CreateWindowExW(0, cls, text, WS_CHILD | WS_VISIBLE | style,
                               x, y, w, h, hwnd_,
                               reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), nullptr, nullptr);
    }

    void BuildUi() {
        HFONT font = reinterpret_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
        auto applyFont = [font](HWND h) {
            SendMessageW(h, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
        };

        applyFont(Make(L"STATIC", L"CLIENT GAME", 0, 18, 16, 120, 22, 0));
        clientCombo_ = Make(WC_COMBOBOXW, L"", CBS_DROPDOWNLIST | WS_VSCROLL,
                            18, 40, 470, 250, IDC_CLIENT);
        applyFont(clientCombo_);
        applyFont(Make(L"BUTTON", L"QUÉT CLIENT", BS_PUSHBUTTON, 500, 39, 165, 29, IDC_SCAN));
        applyFont(Make(L"BUTTON", L"TEST BRIDGE", BS_PUSHBUTTON, 500, 76, 165, 29, IDC_PROBE));

        applyFont(Make(L"STATIC", L"TEST SEMANTIC AUTO FIGHT — KHÔNG GIẢ LẬP CHUỘT",
                       0, 18, 118, 500, 22, 0));
        applyFont(Make(L"BUTTON", L"BẬT AUTO FIGHT (ĐÁNH QUÁI)", BS_DEFPUSHBUTTON,
                       18, 144, 315, 48, IDC_AUTOFIGHT_ON));
        applyFont(Make(L"BUTTON", L"TẮT AUTO FIGHT", BS_PUSHBUTTON,
                       350, 144, 315, 48, IDC_AUTOFIGHT_OFF));

        status_ = Make(L"STATIC", L"RUNTIME: CHƯA TEST", SS_CENTER | SS_CENTERIMAGE | WS_BORDER,
                       18, 208, 647, 48, IDC_STATUS);
        applyFont(status_);

        log_ = Make(L"EDIT",
                    L"v0.1.2 — AutoFight_Main.StartAutoFight(1/0) + Unity-main-thread + frozen-donor guards.\\r\\n",
                    WS_BORDER | ES_MULTILINE | ES_READONLY | WS_VSCROLL | ES_AUTOVSCROLL,
                    18, 270, 647, 80, IDC_LOG);
        applyFont(log_);
        ScanClients();
    }

    void Log(const std::wstring& line) {
        const int length = GetWindowTextLengthW(log_);
        SendMessageW(log_, EM_SETSEL, length, length);
        const std::wstring text = line + L"\r\n";
        SendMessageW(log_, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(text.c_str()));
        SendMessageW(log_, EM_SCROLLCARET, 0, 0);
    }

    void ScanClients() {
        bridge_.Close();
        clients_ = FindClients();
        SendMessageW(clientCombo_, CB_RESETCONTENT, 0, 0);
        for (const auto& client : clients_) {
            const std::wstring label = L"PID " + std::to_wstring(client.pid) + L"  •  " + client.title;
            SendMessageW(clientCombo_, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(label.c_str()));
        }
        if (!clients_.empty()) SendMessageW(clientCombo_, CB_SETCURSEL, 0, 0);
        Log(L"Quét thấy " + std::to_wstring(clients_.size()) + L" client GameAssembly.dll");
    }

    bool EnsureAttach(std::wstring& error) {
        const int index = static_cast<int>(SendMessageW(clientCombo_, CB_GETCURSEL, 0, 0));
        if (index < 0 || index >= static_cast<int>(clients_.size())) {
            error = L"Chưa chọn client";
            return false;
        }
        const GameClient& client = clients_[static_cast<std::size_t>(index)];
        if (bridge_.AttachedTo(client.pid)) return true;
        if (!bridge_.Attach(client, error)) return false;
        Log(L"Đã attach PID " + std::to_wstring(client.pid) + L" bằng AutoFight test bridge");
        return true;
    }

    void Probe() {
        std::wstring error;
        if (!EnsureAttach(error)) {
            SetText(status_, L"BRIDGE: ATTACH FAIL");
            Log(L"PROBE FAIL: " + error);
            return;
        }
        Response response{};
        if (bridge_.Call(Command::Probe, response, error, 1800)) {
            SetText(status_, L"BRIDGE: PASS • CHƯA GỬI ACTION");
            Log(response.detail[0] ? response.detail : L"Bridge probe PASS");
        } else {
            SetText(status_, L"BRIDGE: FAIL");
            Log(L"PROBE FAIL: " + error);
        }
    }

    void SetAutoFight(bool enabled) {
        std::wstring error;
        if (!EnsureAttach(error)) {
            SetText(status_, L"AUTO FIGHT: ATTACH FAIL");
            Log(std::wstring(enabled ? L"ON" : L"OFF") + L" ATTACH FAIL: " + error);
            return;
        }
        Response response{};
        const Command command = enabled ? Command::StartAutoFight : Command::StopAutoFight;
        if (bridge_.Call(command, response, error, 2500)) {
            SetText(status_, enabled
                ? L"AUTO FIGHT ON: LỆNH RETURNED • CHỜ XÁC NHẬN RUNTIME"
                : L"AUTO FIGHT OFF: LỆNH RETURNED • CHỜ XÁC NHẬN RUNTIME");
            Log(std::wstring(enabled ? L"ON: " : L"OFF: ") +
                (response.detail[0] ? response.detail : L"command returned OK"));
        } else {
            SetText(status_, enabled ? L"AUTO FIGHT ON: FAIL" : L"AUTO FIGHT OFF: FAIL");
            Log(std::wstring(enabled ? L"ON FAIL: " : L"OFF FAIL: ") + error);
        }
    }

    LRESULT Handle(UINT msg, WPARAM wp, LPARAM lp) {
        switch (msg) {
            case WM_CREATE:
                BuildUi();
                return 0;
            case WM_COMMAND:
                switch (LOWORD(wp)) {
                    case IDC_SCAN: ScanClients(); break;
                    case IDC_PROBE: Probe(); break;
                    case IDC_AUTOFIGHT_ON: SetAutoFight(true); break;
                    case IDC_AUTOFIGHT_OFF: SetAutoFight(false); break;
                    default: break;
                }
                return 0;
            case WM_DESTROY:
                bridge_.Close();
                PostQuitMessage(0);
                return 0;
            default:
                return DefWindowProcW(hwnd_, msg, wp, lp);
        }
    }

    HWND hwnd_ = nullptr;
    HWND clientCombo_ = nullptr;
    HWND status_ = nullptr;
    HWND log_ = nullptr;
    std::vector<GameClient> clients_;
    BridgeClient bridge_;
};

} // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int show) {
    App app;
    if (!app.Create(instance)) return 2;
    app.Show(show);
    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}
