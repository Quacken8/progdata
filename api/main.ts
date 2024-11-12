async function generateTests(
  hwNum: string,
  fileNum: number,
  testsPerFile: number,
  range: "small" | "full"
) {
  const tmpDir = `sample`;
  await Deno.mkdir(tmpDir, { recursive: true });

  for (let i = 0; i < fileNum; i++) {
    const genProcess = new Deno.Command(`./homeworks/${hwNum}/gen.sh`, {
      args: [testsPerFile.toFixed(0), range],
      stdout: "piped",
    }).spawn();
    await genProcess.stdout.pipeTo(
      Deno.openSync(`${tmpDir}/${i}_in.txt`, {
        write: true,
        create: true,
      }).writable
    );

    const inputFile = await Deno.open(`${tmpDir}/${i}_in.txt`, {
      read: true,
    });
    const outProcess = new Deno.Command(`./homeworks/${hwNum}/${hwNum}.out`, {
      stdin: "piped",
      stdout: "piped",
    }).spawn();
    await inputFile.readable.pipeTo(outProcess.stdin);

    await outProcess.stdout.pipeTo(
      Deno.openSync(`${tmpDir}/${i}_out.txt`, {
        write: true,
        create: true,
      }).writable
    );
  }

  const tarProcess = new Deno.Command("tar", {
    args: ["-cf", "sample.tar", `${tmpDir}`],
  }).spawn();

  await tarProcess.status;

  const tarFile = await Deno.readFile("sample.tar");
  await Deno.remove(tmpDir, { recursive: true });
  await Deno.remove("sample.tar");

  return tarFile;
}

Deno.serve(async (req) => {
  const url = new URL(req.url);
  const hwNum = url.searchParams.get("hwNum")?.padStart(2, "0");
  const fileNum = Number(url.searchParams.get("fileNum"));
  const testsPerFile = Number(url.searchParams.get("testsPerFile"));
  const range = url.searchParams.get("range") as "small" | "full";

  try {
    const tarFile = await generateTests(hwNum!, fileNum, testsPerFile, range);
    return new Response(tarFile, {
      headers: {
        "Content-Disposition": "attachment; filename=sample.tar",
        "Content-Type": "application/x-tar",
      },
    });
  } catch (error) {
    return new Response(`Error: ${(error as Error).message}`, { status: 500 });
  }
});
