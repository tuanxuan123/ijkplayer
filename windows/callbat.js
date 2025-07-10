const { exec } = require('child_process');

argCount = process.argv.length - 2;
console.log(argCount);
if (argCount != 3)
{
  console.error(`nodejs 参数个数不为3`);
  console.log(`示例`);
  console.log(`arg1 vcvars64.bat path  eg. "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Auxiliary\\Build\\vcvars64.bat"`);
  console.log(`arg2 msys2_shell.cmd path  eg. "C:\\msys64\\msys2_shell.cmd"`);
  console.log(`arg3 project pixui_ijkplayer/windows path  eg. "/e/video/pixui_ijkplayer/windows"`);
  console.log(`eg. .\\build-ffmpeg.bat "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Auxiliary\\Build\\vcvars64.bat" "C:\\msys64\\msys2_shell.cmd" "/e/video/pixui_ijkplayer/windows"`);
  return;
}
console.log(process.argv[2]); // 输出 arg1
console.log(process.argv[3]); // 输出 arg2
console.log(process.argv[4]); // 输出 arg3

// 转换arg3
function transformString(str) {
  // 将字符串的第一个字母转换为小写
  str = str.charAt(0).toLowerCase() + str.slice(2);
  
  // 将所有反斜杠`\`替换为正斜杠`/`
  str = str.replace(/\\/g, '/');
  str = '/' + str;
  return str;
}
let projPath = transformString(process.argv[4]);
console.log("after convert ", projPath);
exec(`call ./build-ffmpeg.bat ${process.argv[2]} ${process.argv[3]} "${projPath}"`, (error, stdout, stderr) => {
  if (error) {
    console.error(`执行批处理脚本时出错： ${error}`);
    return;
  }
  console.log(`批处理脚本的输出： ${stdout}`);
});
