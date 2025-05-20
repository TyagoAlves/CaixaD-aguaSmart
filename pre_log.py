import os

# Copia o log do último Wi-Fi da SPIFFS (se existir) para o build_output.log antes do build
try:
    from platformio import fs
    spiffs_path = os.path.join(fs.get_project_dir(), '.pio', 'build', 'nodemcuv2', 'spiffs', 'wifi_last.log')
    log_path = os.path.join(fs.get_project_dir(), 'build_output.log')
    if os.path.exists(spiffs_path):
        with open(spiffs_path, 'r', encoding='utf-8') as src, open(log_path, 'w', encoding='utf-8') as dst:
            dst.write(src.read())
except Exception as e:
    pass

log_path = os.path.join(env['PROJECT_DIR'], 'build_output.log')
if os.path.exists(log_path):
    with open(log_path, 'r', encoding='utf-8') as f:
        print('\n===== LOG ANTERIOR DO BUILD =====')
        print(f.read())
        print('===== FIM DO LOG ANTERIOR =====\n')
