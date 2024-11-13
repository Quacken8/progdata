#!/usr/bin/env deno run --allow-net --allow-read --allow-write --allow-env

async function generateTests(
  hwNum: string,
  fileNum: number,
  testsPerFile: number,
  range: "small" | "full"
) {
  const tmpDir = `../tmp`;
  await Deno.mkdir(tmpDir, { recursive: true });

  for (let i = 0; i < fileNum; i++) {
    const genProcess = new Deno.Command(`api/homeworks/${hwNum}/gen.sh`, {
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
    const outProcess = new Deno.Command(`api/homeworks/${hwNum}/${hwNum}.out`, {
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
    args: ["-cf", `${tmpDir}/sample.tar`, `${tmpDir}`],
  }).spawn();

  await tarProcess.status;

  const tarFile = await Deno.readFile(`${tmpDir}/sample.tar`);
  await Deno.remove(tmpDir, { recursive: true });

  return tarFile;
}

export default async (req: Request) => {
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
    const fs = [...Deno.readDirSync(".")].map((f) => f.name).join("\n");
    return new Response(fs, { status: 500 });
  }
};
