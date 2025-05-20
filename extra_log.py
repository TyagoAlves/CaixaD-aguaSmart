import os

def after_build(source, target, env):
    log_path = os.path.join(env['PROJECT_DIR'], 'build_output.log')
    with open(log_path, 'w', encoding='utf-8') as f:
        for t in target:
            if os.path.exists(str(t)):
                f.write(f'Arquivo gerado: {t}\n')
        f.write('\nBuild finalizado.\n')

Import("env")
env.AddPostAction("buildprog", after_build)
