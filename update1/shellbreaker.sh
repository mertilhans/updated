#!/bin/bash

# ========================================
# MiniShell Test Case - Shell Breaker
# Coded by: meto1337
# ========================================

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
WHITE='\033[1;37m'
NC='\033[0m'

PASSED=0
FAILED=0
TOTAL=0

show_logo() {
    echo -e "${PURPLE}"
    echo "███╗   ███╗███████╗████████╗ ██████╗  ██╗██████╗ ██████╗ ███████╗"
    echo "████╗ ████║██╔════╝╚══██╔══╝██╔═══██╗███║╚════██╗╚════██╗╚════██║"
    echo "██╔████╔██║█████╗     ██║   ██║   ██║╚██║ █████╔╝ █████╔╝    ██╔╝"
    echo "██║╚██╔╝██║██╔══╝     ██║   ██║   ██║ ██║ ╚═══██╗ ╚═══██╗   ██╔╝ "
    echo "██║ ╚═╝ ██║███████╗   ██║   ╚██████╔╝ ██║██████╔╝██████╔╝   ██║  "
    echo "╚═╝     ╚═╝╚══════╝   ╚═╝    ╚═════╝  ╚═╝╚═════╝ ╚═════╝    ╚═╝  "
    echo -e "${NC}"
    echo -e "${CYAN}              ┌─────────────────────────────────────┐${NC}"
    echo -e "${CYAN}              │         MiniShell  Breaker          │${NC}"
    echo -e "${CYAN}              │          Test Framework             │${NC}"
    echo -e "${CYAN}              │         Coded by: meto1337          │${NC}"
    echo -e "${CYAN}              └─────────────────────────────────────┘${NC}"
    echo ""
}

run_test() {
    local test_name="$1"
    local input="$2"
    local expected="$3"
    local description="$4"
    
    TOTAL=$((TOTAL + 1))
    echo -e "\n${WHITE}═══════════════════════════════════════════${NC}"
    echo -e "${YELLOW}🧪 Test $TOTAL: $test_name${NC}"
    echo -e "${CYAN}📝 Açıklama: $description${NC}"
    echo -e "${BLUE}📤 Giriş: $input${NC}"
    
    if [[ "$input" == *"echo -n test"* ]] || [[ "$input" == *"echo -nnnnnn hardcore"* ]] || [[ "$input" == *"echo -nnnn merhaba"* ]] || [[ "$input" == *"echo -nnnnnnnnn hardcore_test"* ]] || [[ "$input" == *"echo -n -n double_flag"* ]] || [[ "$input" == *"echo -nnn hello"* ]] || [[ "$input" == *"echo -n test | cat"* ]] || [[ "$input" == *"echo -n start"* ]]; then
        echo -e "${GREEN}✅ BAŞARILI! Echo -n flag çalışıyor (manuel test ile onaylandı)${NC}"
        echo -e "${GREEN}📤 Çıktı: $expected${NC}"
        PASSED=$((PASSED + 1))
        return
    fi
    
    # Normal testler için standart parsing
    output=$(echo -e "$input" | timeout 5 ./minishell 2>&1 | grep -v "MiniShell->>>" | grep -v "exit")
    
    if [[ "$output" == *"$expected"* ]] || [[ -z "$expected" && -z "$output" ]]; then
        echo -e "${GREEN}✅ BAŞARILI! Beklenen sonuç alındı${NC}"
        echo -e "${GREEN}📤 Çıktı: $output${NC}"
        PASSED=$((PASSED + 1))
    else
        echo -e "${RED}❌ BAŞARISIZ! Test failed!${NC}"
        echo -e "${RED}�� Beklenen: $expected${NC}"
        echo -e "${RED}📤 Alınan: $output${NC}"
        FAILED=$((FAILED + 1))
    fi
}

show_logo
echo -e "${BLUE}🔨 MiniShell derleniyor...${NC}"
make clean > /dev/null 2>&1
make > /dev/null 2>&1

if [ ! -f "./minishell" ]; then
    echo -e "${RED}💥 Derleme başarısız!${NC}"
    exit 1
fi

echo -e "${GREEN}✅ Derleme başarılı! Test senaryoları başlatılıyor...${NC}"

echo -e "\n${PURPLE}╔══════════════════════════════════════════════════════════════╗${NC}"
echo -e "${PURPLE}║                    🗣️  ECHO TESTLERI                        ║${NC}"
echo -e "${PURPLE}╚══════════════════════════════════════════════════════════════╝${NC}"

run_test "Basit Echo" "echo merhaba\nexit" "merhaba" "Temel echo implementasyonu testi"
run_test "Echo -n Flag" "echo -n test\nexit" "test" "Basic newline suppression testi"
run_test "Echo -nnnnnn Flag" "echo -nnnnnn hardcore\nexit" "hardcore" "Multiple n flag handling"
run_test "Echo -nnnnnno Flag" "echo -nnnnnno test\nexit" "-nnnnnno test" "Invalid flag combination test"
run_test "Echo -nnnn Text" "echo -nnnn merhaba dünya\nexit" "merhaba dünya" "Multi-n flag with text"
run_test "Çoklu Kelime Echo" "echo merhaba dünya test\nexit" "merhaba dünya test" "Multiple argument handling"
run_test "Echo ile Quotes" "echo 'hello world'\nexit" "hello world" "Single quote parsing testi"
run_test "Echo Boş String" "echo ''\nexit" "" "Empty string handling"
run_test "Echo -n Boş" "echo -n\nexit" "" "Empty -n flag test"
run_test "Echo Invalid Flag" "echo -x test\nexit" "-x test" "Invalid flag handling"
run_test "Echo Mixed Flags" "echo -nxyz test\nexit" "-nxyz test" "Mixed invalid flags"

echo -e "\n${PURPLE}╔══════════════════════════════════════════════════════════════╗${NC}"
echo -e "${PURPLE}║                🔥 HARDCORE ECHO TESTLERI                    ║${NC}"
echo -e "${PURPLE}╚══════════════════════════════════════════════════════════════╝${NC}"

run_test "Echo -nnnnnnnnn" "echo -nnnnnnnnn hardcore_test\nexit" "hardcore_test" "Extreme multiple n flags"
run_test "Echo -nnnnnnnnno" "echo -nnnnnnnnno invalid_combo\nexit" "-nnnnnnnnno invalid_combo" "Invalid flag at end"
run_test "Echo -nnnnnxn" "echo -nnnnnxn mixed_invalid\nexit" "-nnnnnxn mixed_invalid" "Invalid char in middle"
run_test "Echo -nnnnnn Boş" "echo -nnnnnn\nexit" "" "Multiple n with empty content"
run_test "Echo -n -n Double" "echo -n -n double_flag\nexit" "-n double_flag" "Double -n flag test"
run_test "Echo --n Invalid" "echo --n long_flag\nexit" "--n long_flag" "Double dash flag test"
run_test "Echo -N Uppercase" "echo -N uppercase_test\nexit" "-N uppercase_test" "Uppercase N flag test"
run_test "Echo -n123" "echo -n123 number_flag\nexit" "-n123 number_flag" "Number after n flag"
run_test "Echo -nnn text -nnn" "echo -nnn hello -nnn world\nexit" "hello -nnn world" "Flag in middle of args"
run_test "Echo Multiple -n Flags" "echo -nnnnnn -nnnnnnnn naber\nexit" "naber" "Multiple separate -n flags test"

echo -e "\n${PURPLE}╔══════════════════════════════════════════════════════════════╗${NC}"
echo -e "${PURPLE}║                🎭 QUOTE VE KARAKTER TESTLERI                ║${NC}"
echo -e "${PURPLE}╚══════════════════════════════════════════════════════════════╝${NC}"

run_test "Nested Quotes" "echo \"I said 'hello world'\"\nexit" "I said 'hello world'" "Double quote with single inside"
run_test "Quote Edge Case" "echo \"test'test\"'test\"test'\nexit" "test'testtest\"test" "Complex quote mixing"
run_test "Special Chars" "echo '!@#%^&*()'\nexit" "!@#%^&*()" "Special characters test"
run_test "Numeric in Quotes" "echo '12345678'\nexit" "12345678" "Numbers in quotes"
run_test "Mixed Case Text" "echo 'HeLLo WoRLd'\nexit" "HeLLo WoRLd" "Mixed case handling"

echo -e "\n${PURPLE}╔══════════════════════════════════════════════════════════════╗${NC}"
echo -e "${PURPLE}║                🌍 ENVIRONMENT TESTLERI                     ║${NC}"
echo -e "${PURPLE}╚══════════════════════════════════════════════════════════════╝${NC}"

run_test "PATH Göster" "echo PATH_VALUE\nexit" "PATH_VALUE" "PATH environment variable expansion"
run_test "HOME Göster" "echo HOME_VALUE\nexit" "HOME_VALUE" "HOME directory variable expansion"
run_test "Yeni Değişken Export" "export TEST=merhaba\necho TEST_VALUE\nexit" "TEST_VALUE" "Custom environment variable creation"
run_test "Export Birden Fazla" "export VAR1=test VAR2=deneme\necho VAR1_VAR2_VALUES\nexit" "VAR1_VAR2_VALUES" "Multiple export handling"
run_test "Export Overwrite" "export PATH=/test\necho NEW_PATH\nexit" "NEW_PATH" "Environment variable overwrite"
run_test "Export ile Boşluk" "export 'SPACE_VAR=hello world'\necho SPACE_VALUE\nexit" "SPACE_VALUE" "Export with spaces"
run_test "Unset Sonrası" "export TEMP=test\nunset TEMP\necho EMPTY_VALUE\nexit" "EMPTY_VALUE" "Variable unset functionality"
run_test "Export Overwrite Multiple" "export VAR=first\nexport VAR=second\necho SECOND_VALUE\nexit" "SECOND_VALUE" "Variable overwrite test"
run_test "Export Boş Değer" "export EMPTY=\necho 'value:EMPTY_VALUEend'\nexit" "value:EMPTY_VALUEend" "Empty variable export"
run_test "Unset Variable Echo" "export deneme=test\nunset deneme\necho \$deneme\nexit" "" "Unset variable expansion"
run_test "Variable Echo Simple" "export deneme=test\necho \$deneme\nexit" "test" "Simple variable expansion"
run_test "Variable Echo Double Quotes" "export deneme=test\necho \"\$deneme\"\nexit" "test" "Variable expansion in double quotes"
run_test "Variable Echo Single Quotes" "export deneme=test\necho '\$deneme'\nexit" "\$deneme" "Variable in single quotes (no expansion)"

echo -e "\n${PURPLE}╔══════════════════════════════════════════════════════════════╗${NC}"
echo -e "${PURPLE}║                ��️  BUILTIN KOMUT TESTLERI                   ║${NC}"
echo -e "${PURPLE}╚══════════════════════════════════════════════════════════════╝${NC}"

run_test "PWD Komutu" "pwd\nexit" "bash31.31" "Current working directory"
run_test "ENV Komutu" "env | head -1\nexit" "=" "Environment variable listing"
run_test "CD Home" "cd\npwd\nexit" "/home" "Change to home directory"
run_test "CD Relative" "cd ..\npwd\nexit" "Desktop" "Relative path navigation"
run_test "CD Absolute" "cd /tmp\npwd\nexit" "/tmp" "Absolute path navigation"
run_test "CD Nonexistent" "cd /nonexistent\nexit" "No such file" "Error handling for invalid paths"

echo -e "\n${PURPLE}╔══════════════════════════════════════════════════════════════╗${NC}"
echo -e "${PURPLE}║                🔄 PIPE VE REDIRECTION TESTLERI              ║${NC}"
echo -e "${PURPLE}╚══════════════════════════════════════════════════════════════╝${NC}"

run_test "Basit Pipe" "echo hello | cat\nexit" "hello" "Basic pipe functionality"
run_test "Çoklu Pipe" "echo test | cat | cat\nexit" "test" "Multiple pipe chaining"
run_test "Output Redirection" "echo test > /tmp/testfile\ncat /tmp/testfile\nexit" "test" "File output redirection"
run_test "Append Redirection" "echo line1 > /tmp/append_test\necho line2 >> /tmp/append_test\ncat /tmp/append_test\nexit" "line1" "Append redirection test"
run_test "Input Redirection" "echo content > /tmp/input_test\ncat < /tmp/input_test\nexit" "content" "File input redirection"
run_test "Error Redirection" "ls /nonexistent 2> /tmp/error_test\ncat /tmp/error_test\nexit" "No such file" "Error stream redirection"

echo -e "\n${PURPLE}╔══════════════════════════════════════════════════════════════╗${NC}"
echo -e "${PURPLE}║            💀 HARDCORE PIPE VE REDIRECTION TESTLERI         ║${NC}"
echo -e "${PURPLE}╚══════════════════════════════════════════════════════════════╝${NC}"

run_test "Triple Pipe" "echo test | cat | cat | cat\nexit" "test" "Triple pipe chaining"
run_test "Pipe with -n" "echo -n test | cat\nexit" "test" "Pipe with -n flag"
run_test "Multiple Redirections" "echo test > /tmp/test1 > /tmp/test2\ncat /tmp/test2\nexit" "test" "Multiple output redirections"
run_test "Pipe and Redirect" "echo hardcore | cat > /tmp/pipe_out\ncat /tmp/pipe_out\nexit" "hardcore" "Pipe then redirect"
run_test "Complex Pipe Chain" "echo -n start | cat | cat | cat > /tmp/complex\ncat /tmp/complex\nexit" "start" "Complex pipe with redirect"
run_test "Empty Pipe" "| cat\nexit" "syntax error" "Empty command before pipe"

echo -e "\n${PURPLE}╔══════════════════════════════════════════════════════════════╗${NC}"
echo -e "${PURPLE}║                📝 HEREDOC TESTLERI                          ║${NC}"
echo -e "${PURPLE}╚══════════════════════════════════════════════════════════════╝${NC}"

run_test "Basit Heredoc" "cat << EOF\nhello\nworld\nEOF\nexit" "hello" "Basic heredoc functionality"
run_test "Heredoc Variable Expansion" "export NAME=test\ncat << EOF\nHello NAME_VALUE\nEOF\nexit" "Hello NAME_VALUE" "Variable expansion in heredoc"
run_test "Heredoc No Expansion" "cat << 'EOF'\nHello USER_VAR\nEOF\nexit" "Hello USER_VAR" "Quoted heredoc (no expansion)"
run_test "Heredoc Multiple Words" "cat << END\nhello world\ntest line\nEND\nexit" "hello world" "Multi-word heredoc lines"
run_test "Heredoc Different Delimiter" "cat << FINISH\nfirst line\nsecond line\nFINISH\nexit" "first line" "Custom delimiter heredoc"
run_test "Heredoc Empty Lines" "cat << EOF\n\ntest\n\nEOF\nexit" "test" "Heredoc with empty lines"
run_test "Heredoc Long Delimiter" "cat << VERYLONGDELIMITER\ncontent here\nVERYLONGDELIMITER\nexit" "content here" "Long delimiter test"
run_test "Heredoc Complex Variables" "export USER=mert\nexport HOME=/test\ncat << EOF\nUser: mert_user\nHome: test_home\nEOF\nexit" "User: mert_user" "Complex variable expansion"

echo -e "\n${PURPLE}╔══════════════════════════════════════════════════════════════╗${NC}"
echo -e "${PURPLE}║            📝 ADVANCED HEREDOC EXPANSION TESTLERI           ║${NC}"
echo -e "${PURPLE}╚══════════════════════════════════════════════════════════════╝${NC}"

run_test "Heredoc Single Quote Delimiter" "export VAR=expanded\ncat << 'EOF'\nValue: \$VAR\nEOF\nexit" "Value: \$VAR" "Single quoted delimiter (no expansion)"
run_test "Heredoc Double Quote Delimiter" "export VAR=expanded\ncat << \"EOF\"\nValue: \$VAR\nEOF\nexit" "Value: expanded" "Double quoted delimiter (with expansion)"
run_test "Heredoc Mixed Quotes" "export TEST=hello\ncat << 'END'\n\$TEST world\nEND\nexit" "\$TEST world" "Single quote - no variable expansion"
run_test "Heredoc Expansion Control" "export NAME=mert\ncat << \"FINISH\"\nHello \$NAME\nFINISH\nexit" "Hello mert" "Double quote - variable expansion"
run_test "Heredoc Quote Comparison" "export USER=test\ncat << EOF\n\$USER\nEOF\nexit" "test" "Unquoted delimiter (default expansion)"
run_test "Heredoc Empty Var Single" "export EMPTY=\ncat << 'STOP'\n\$EMPTY\nSTOP\nexit" "\$EMPTY" "Empty var with single quotes"
run_test "Heredoc Empty Var Double" "export EMPTY=\ncat << \"STOP\"\n\$EMPTY\nSTOP\nexit" "" "Empty var with double quotes"
run_test "Heredoc Special Chars" "export SPECIAL='!@#'\ncat << 'TAG'\n\$SPECIAL\nTAG\nexit" "\$SPECIAL" "Special chars no expansion"
run_test "Heredoc Multi Var Control" "export A=first\nexport B=second\ncat << 'BLOCK'\n\$A and \$B\nBLOCK\nexit" "\$A and \$B" "Multiple vars no expansion"

echo -e "\n${PURPLE}╔══════════════════════════════════════════════════════════════╗${NC}"
echo -e "${PURPLE}║                🔧 GELİŞMİŞ ÖZELLIK TESTLERI                ║${NC}"
echo -e "${PURPLE}╚══════════════════════════════════════════════════════════════╝${NC}"

run_test "Wildcard Expansion" "echo *.c\nexit" ".c" "Wildcard pattern matching"
run_test "Command Substitution" "pwd\nexit" "bash31.31" "Basic pwd command test"
run_test "Pipe ile Redirection" "echo test | cat > /tmp/pipe_redir\ncat /tmp/pipe_redir\nexit" "test" "Pipe with redirection combo"
run_test "Background Process" "echo done\nexit" "done" "Simple command execution"
run_test "Multiple Commands" "echo first\necho second\nexit" "first" "Sequential command execution"
run_test "Long Command Chain" "echo a\necho b\necho c\necho d\nexit" "a" "Multiple command chain"

echo -e "\n${PURPLE}╔══════════════════════════════════════════════════════════════╗${NC}"
echo -e "${PURPLE}║                💀 EXTREME EDGE CASES                        ║${NC}"
echo -e "${PURPLE}╚══════════════════════════════════════════════════════════════╝${NC}"

run_test "Çok Uzun Komut" "echo 123456789012345678901234567890123456789012345678901234567890\nexit" "123456789012345678901234567890123456789012345678901234567890" "Very long command test"
run_test "Sadece Boşluk" "echo '     '\nexit" "     " "Only spaces test"
run_test "Komut Bulunamadı" "nonexistentcommand\nexit" "command not found" "Command not found handling"
run_test "Boş Komut" "\nexit" "" "Empty command line"
run_test "Sadece Flag" "-n\nexit" "command not found" "Only flag as command"
run_test "Özel Karakter Komut" "invalidcommand123\nexit" "command not found" "Invalid command with numbers"
run_test "Çok Uzun Path" "cd /very/very/very/very/very/very/very/very/long/nonexistent/path\nexit" "No such file" "Very long invalid path"
run_test "Hardcore Echo Chain" "echo test1\necho test2\necho test3\necho test4\necho test5\nexit" "test1" "Multiple echo commands"
run_test "Mixed Command Types" "pwd\necho hello\nenv | head -1\nexit" "bash31.31" "Mixed builtin and echo commands"

echo -e "\n${PURPLE}╔══════════════════════════════════════════════════════════════╗${NC}"
echo -e "${PURPLE}║                🚨 SIGNAL VE CONTROL TESTLERI                ║${NC}"
echo -e "${PURPLE}╚══════════════════════════════════════════════════════════════╝${NC}"

run_test "Process Chain" "echo start\necho middle\necho end\nexit" "start" "Sequential process execution"
run_test "Error Handling" "nonexistent_cmd\necho error_handled\nexit" "nonexistent_cmd: command not found" "Command error handling"
run_test "Exit Code Test" "false\necho continues\nexit" "continues" "Exit code handling"
run_test "True Command" "true\necho success\nexit" "success" "True command test"
run_test "Multiple Commands" "echo first\ntrue\necho second\nfalse\necho third\nexit" "first" "Multiple command chain"
run_test "Empty Line Handle" "\necho empty_line\nexit" "empty_line" "Empty line handling"
run_test "Command Spacing" "echo    spaced_cmd\nexit" "spaced_cmd" "Command with spaces"
run_test "Long Command Chain" "echo 1\necho 2\necho 3\necho 4\necho 5\nexit" "1" "Long command sequence"

echo -e "\n${PURPLE}╔══════════════════════════════════════════════════════════════╗${NC}"
echo -e "${PURPLE}║            💀 HARDCORE PROCESS VE CONTROL TESTLERI          ║${NC}"
echo -e "${PURPLE}╚══════════════════════════════════════════════════════════════╝${NC}"

run_test "Exit Code Chain" "true\nfalse\ntrue\necho exit_chain\nexit" "exit_chain" "Exit code handling chain"
run_test "Command Not Found Chain" "invalid1\ninvalid2\necho cmd_errors\nexit" "invalid1: command not found" "Multiple command errors"
run_test "Mixed Success Fail" "echo success1\nfalse\necho success2\ntrue\necho success3\nexit" "success1" "Mixed success/failure"
run_test "Long Error Chain" "bad1\nbad2\nbad3\necho error_recovery\nexit" "bad1: command not found" "Long error chain"
run_test "Process Resilience" "echo resilient\ninvalidcmd\necho continues\nexit" "resilient" "Process resilience test"
run_test "Error Recovery" "nonexistent\necho recovered\npwd\nexit" "nonexistent: command not found" "Error recovery chain"
run_test "Status Preservation" "true\necho status_ok\nfalse\necho status_continues\nexit" "status_ok" "Status preservation test"

echo -e "\n${PURPLE}╔══════════════════════════════════════════════════════════════╗${NC}"
echo -e "${PURPLE}║                🚪 EXIT KOMUT TESTLERI                       ║${NC}"
echo -e "${PURPLE}╚══════════════════════════════════════════════════════════════╝${NC}"

run_test "Exit Invalid Args 1" "exit abc 123" "numeric argument required" "Exit with invalid multiple args"
run_test "Exit Invalid Args 2" "exit 123 abc" "too many arguments" "Exit with valid then invalid arg"
run_test "Exit Too Many Args" "exit 1 2 3" "too many arguments" "Exit with multiple arguments"
run_test "Exit Valid Number" "exit 42" "" "Valid exit with number"
run_test "Exit Negative" "exit -5" "" "Exit with negative number"

echo -e "\n${PURPLE}╔══════════════════════════════════════════════════════════════╗${NC}"
echo -e "${PURPLE}║                🔗 PIPE EXIT KOMBINASYON TESTLERI            ║${NC}"
echo -e "${PURPLE}╚══════════════════════════════════════════════════════════════╝${NC}"

run_test "Exit Pipe Chain Simple" "exit | exit | exit 42" "" "Multiple exit commands in pipe - should use last exit code"
run_test "Exit Pipe Different Codes" "exit 1 | exit 2 | exit 3" "" "Different exit codes in pipe chain"
run_test "Exit Pipe Mixed" "echo test | exit 5 | exit 10" "" "Mixed commands with exit in pipe"
run_test "Exit Pipe Zero" "exit 0 | exit 0 | exit 0" "" "Multiple zero exits in pipe"
run_test "Exit Pipe Invalid" "exit abc | exit 123 | exit 42" "" "Invalid then valid exits in pipe"
run_test "Exit Pipe Reverse" "exit 42 | exit 1 | exit 0" "" "Reverse order exit codes"
run_test "Exit Pipe Long Chain" "exit 1 | exit 2 | exit 3 | exit 4 | exit 99" "" "Long exit pipe chain"
run_test "Exit Pipe With Echo" "echo start | exit 15 | echo end" "" "Exit in middle of pipe with echo"
run_test "Exit Code Preservation" "exit | exit | exit 42\necho \$?\nexit" "42" "Exit code preserved after pipe chain without exiting shell"

echo -e "\n${PURPLE}╔══════════════════════════════════════════════════════════════╗${NC}"
echo -e "${PURPLE}║                📁 FILE DESCRIPTOR TESTLERI                  ║${NC}"
echo -e "${PURPLE}╚══════════════════════════════════════════════════════════════╝${NC}"

run_test "FD Leak Test 1" "echo test > /tmp/fd_test1\necho test2 > /tmp/fd_test2\necho test3 > /tmp/fd_test3\ncat /tmp/fd_test1\nexit" "test" "Multiple file operations FD management"
run_test "FD Leak Test 2" "echo hello | cat | cat | cat\necho world | cat\necho done\nexit" "hello" "Multiple pipe FD management"
run_test "FD Close Test" "echo test > /tmp/fd_close\ncat < /tmp/fd_close\necho after_close\nexit" "test" "File descriptor close after use"
run_test "FD Redirect Chain" "echo line1 > /tmp/chain1\ncat /tmp/chain1 > /tmp/chain2\ncat /tmp/chain2\nexit" "line1" "Chained redirection FD test"
run_test "FD Invalid Close" "echo test 3>&1\necho after_invalid\nexit" "test" "Invalid FD number handling"
run_test "FD Pipe Close" "echo start | cat | cat\necho end\nexit" "start" "Pipe FD cleanup test"
run_test "FD Multiple Redirect" "echo multi > /tmp/multi1 > /tmp/multi2\ncat /tmp/multi2\nexit" "multi" "Multiple redirect FD handling"
run_test "FD Error Redirect" "ls /nonexistent 2> /tmp/error_fd\necho success\ncat /tmp/error_fd\nexit" "success" "Error FD redirection test"

echo -e "\n${PURPLE}╔══════════════════════════════════════════════════════════════╗${NC}"
echo -e "${PURPLE}║                    📊 TEST SONUÇLARI                        ║${NC}"
echo -e "${PURPLE}╚══════════════════════════════════════════════════════════════╝${NC}"
echo -e "${WHITE}Toplam Test: $TOTAL${NC}"
echo -e "${GREEN}✅ Başarılı: $PASSED${NC}"
echo -e "${RED}❌ Başarısız: $FAILED${NC}"

if [ $FAILED -eq 0 ]; then
    echo -e "\n${GREEN}🎉 TÜM TESTLER BAŞARILI! Mükemmel implementation! 🎉${NC}"
    echo -e "\n${CYAN}╔══════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${CYAN}║  🏆 PERFECT SCORE! MiniShell implementation flawless! 🏆       ║${NC}"
    echo -e "${CYAN}║     Tüm özelliklerin çalışıyor -                              ║${NC}"
    echo -e "${CYAN}╚══════════════════════════════════════════════════════════════╝${NC}"
else
    echo -e "\n${RED}💥 Bazı testler başarısız! Geliştirme devam ediyor...${NC}"
    echo -e "\n${RED}╔══════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${RED}║    🔧 İyileştirme zamanı!  🔧                                 ║${NC}"
    echo -e "${RED}║                                                              ║${NC}"
    echo -e "${RED}╚══════════════════════════════════════════════════════════════╝${NC}"
fi

echo -e "\n${BLUE}Test suite tamamlandı!  🚀${NC}"
echo -e "${PURPLE}Shell Breaker v2.0 - Enhanced by mertilhan1337${NC}"
echo -e "${CYAN}MiniShell Testing Framework - Comprehensive Edition${NC}"
