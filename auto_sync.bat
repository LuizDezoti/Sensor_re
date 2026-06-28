@echo off
echo ==============================================
echo  Sincronizacao Automatica do GitHub Ativada!
echo  (Deixe esta janela aberta. Aperte Ctrl+C para parar)
echo ==============================================

:loop
REM Pega o status do git para um arquivo temporario
git status --porcelain > changes.tmp

REM Verifica o tamanho do arquivo (se for maior que 0 bytes, teve mudanca)
for %%A in (changes.tmp) do (
    if %%~zA GTR 0 (
        echo.
        echo [ %time% ] Mudancas detectadas! Salvando e enviando para o GitHub...
        git add .
        git commit -m "auto: backup automatico"
        git push
        echo [ %time% ] Sincronizacao concluida com sucesso!
    )
)

del changes.tmp
REM Espera 30 segundos silenciosamente antes de verificar de novo
timeout /t 30 /nobreak > nul
goto loop
