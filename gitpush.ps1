# Script para automatizar commits e push no GitHub
param(
    [string]$msg = "Alterações automáticas"
)

# Mostra status antes de tudo
Write-Host "Status do repositório:" -ForegroundColor Cyan
& git status

# Adiciona todos os arquivos
Write-Host "Adicionando arquivos..." -ForegroundColor Yellow
& git add .

# Faz commit
Write-Host "Realizando commit..." -ForegroundColor Yellow
& git commit -m "$msg"

# Faz push para o repositório remoto
Write-Host "Enviando para o GitHub..." -ForegroundColor Green
& git push

Write-Host "Pronto! Alterações salvas e enviadas para o GitHub." -ForegroundColor Green

# Dicas rápidas
Write-Host "\nDicas rápidas de branches:" -ForegroundColor Cyan
Write-Host "Criar novo branch: git checkout -b nome-do-branch" -ForegroundColor Gray
Write-Host "Trocar de branch: git checkout main" -ForegroundColor Gray
Write-Host "Mesclar branch: git merge nome-do-branch" -ForegroundColor Gray
Write-Host "Enviar branch: git push -u origin nome-do-branch" -ForegroundColor Gray
