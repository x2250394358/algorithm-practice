const diskInput = document.querySelector("#diskInput");

const towerA = document.querySelector("#towerA");
const towerB = document.querySelector("#towerB");
const towerC = document.querySelector("#towerC");
const nextButton = document.querySelector("#nextButton");
const resetButton = document.querySelector("#resetButton");
const previousButton = document.querySelector("#prevButton");
const curStep = document.querySelector("#currentStep");
const totalStep = document.querySelector("#totalSteps");
const autoplayButton = document.querySelector("#autoplay");
const diskLayer = document.querySelector(".disk-layer");


const moves = [];

let towerState = {
    A:[],
    B:[],
    C:[],
};

let currentStep = 0;
let autoplayTimer = null;
let isAnimating = false;

function getTowerX(tower)
{
    const towerRect = tower.getBoundingClientRect();
    const layerRect = diskLayer.getBoundingClientRect();

    return towerRect.left
        + towerRect.width / 2
        - layerRect.left;
}

function getDiskBottom(level)
{
    return 30 + level * 19;
}

function createDisk(diskNumber, tower, level)
{
    const disk = document.createElement("div");

    disk.classList.add("disk");

    disk.dataset.disk = diskNumber;

    disk.style.width = `${40 + diskNumber * 25}px`;

    const x = getTowerX(tower);

    disk.style.left = `${x}px`;

    disk.style.bottom = `${30 + level * 19}px`;

    diskLayer.appendChild(disk);
}
function findDisk(diskNumber)
{
    return document.querySelector(
        `.disk[data-disk="${diskNumber}"]`
    );
}

function getMoveInfo(move)
{
    const sourceLevel = towerState[move.from].length - 1;
    const targetLevel = towerState[move.to].length;

    return {
        sourceLevel: sourceLevel,
        targetLevel: targetLevel
    };
}

function animateMove(move)
{
    const disk = findDisk(move.disk);

    if (!disk) return;

    const info = getMoveInfo(move);

    const targetTower = {
        A: towerA,
        B: towerB,
        C: towerC
    }[move.to];

    // ① 上升
    disk.style.bottom = "180px";

    disk.addEventListener("transitionend", function upHandler(event)
    {
        if (event.propertyName !== "bottom") return;

        disk.removeEventListener("transitionend", upHandler);

        // ② 水平移动
        const x = getTowerX(targetTower);

        disk.style.left = `${x}px`;

        disk.addEventListener("transitionend", function moveHandler(event)
        {
            if (event.propertyName !== "left") return;

            disk.removeEventListener("transitionend", moveHandler);

            // ③ 下降到目标层
            disk.style.bottom =
    `${getDiskBottom(info.targetLevel)}px`;

disk.addEventListener("transitionend", function downHandler(event)
{
    if (event.propertyName !== "bottom") return;

    disk.removeEventListener("transitionend", downHandler);

    // 动画全部完成，现在才更新数据
    executeMove(move);

currentStep++;

curStep.textContent = currentStep;

isAnimating = false;

console.log("动画完成");
console.log(towerState);
});
        });
    });
}

function moveDiskToTower(diskNumber, tower)
{
    const disk = findDisk(diskNumber);

    if (!disk) return;

    const x = getTowerX(tower);

    disk.style.left = `${x}px`;
}

function getTowerCenter(tower)
{
    const rect = tower.getBoundingClientRect();

    return {
        x: rect.left + rect.width / 2,
        y: rect.top + rect.height / 2
    };
}

function generateMoves(n, from, auxi, to)
{
    if(n == 1)
    {
        moves.push({
            disk:1,
            from:from,
            to:to
        });
        return;
    }
    generateMoves(n-1, from, to, auxi);
    moves.push({
        disk:n,
        from:from,
        to:to
    });
    generateMoves(n-1, auxi, from, to);
}

function resetTowerState(n)
{
    towerState.A = [];
    towerState.B = [];
    towerState.C = [];
    for(let i = n; i >= 1; i--)
    {
        towerState.A.push(i);
    }
}

function executeMove(move)
{
    const disk = towerState[move.from].pop();
    towerState[move.to].push(disk);
}

function unDoMove(move)
{
    const disk = towerState[move.to].pop();
    towerState[move.from].push(disk);
}
function nextStep()
{
    if (currentStep >= moves.length) return;

    if (isAnimating) return;

    const move = moves[currentStep];

    isAnimating = true;

    animateMove(move);
}
function preStep()
{
    if(currentStep <= 0)
    {
        return;
    }
    currentStep--;
    const move = moves[currentStep];
    unDoMove(move);
    render();


}

function stopAutoplay()
{
    if(autoplayTimer !== null)
    {
        clearInterval(autoplayTimer);
        autoplayTimer = null;
    }

    autoplayButton.textContent = "播放";
}

function toggleAutoplay()
{
    if(autoplayTimer !== null)
    {
        stopAutoplay();
        return;
    }

    if(currentStep >= moves.length)
    {
        resetGame();
    }

    autoplayButton.textContent = "暂停";
    autoplayTimer = setInterval(function ()
    {
        nextStep();
        if(currentStep >= moves.length)
        {
            stopAutoplay();
        }
    }, 600);
}

function resetGame()
{
    stopAutoplay();
    const n = Number(diskInput.value);
    // const disks = document.querySelectorAll(".disk");

    // for(const disk of disks)
    // {
    //     disk.remove();
    // }
    // 清空旧的移动步骤
    moves.length = 0;

    // 当前步骤归零
    currentStep = 0;

    

    // 重置三根柱子的状态
    resetTowerState(n);

    // 重新生成移动步骤
    generateMoves(n, "A", "B", "C");

    render()

}
function render()
{
    // 1. 删除页面上原来的所有盘子
    const disks = document.querySelectorAll(".disk");

    for (const disk of disks)
    {
        disk.remove();
    }


    // 2. 定义柱子对应的 DOM 元素
    const towers = {
        A: towerA,
        B: towerB,
        C: towerC
    };


    // 3. 遍历三根柱子
    for (const name of ["A", "B", "C"])
    {
        const tower = towers[name];

        // 当前柱子的状态
        const state = towerState[name];


        // 4. 从数组顶部开始读取
       for (let i = 0; i < state.length; i++)
{
    const diskNumber = state[i];

    const level = i;

    createDisk(diskNumber, tower, level);
}
    }
    curStep.textContent = currentStep;
    totalStep.textContent = moves.length;
}

resetGame();

console.log(moves);


diskInput.addEventListener("change", function(){
    resetGame();
})
nextButton.addEventListener("click", function(){
    stopAutoplay();
    nextStep();
    
})
resetButton.addEventListener("click", function ()
{
    resetGame();
});
previousButton.addEventListener("click", function ()
{
    stopAutoplay();
    preStep();
});
autoplayButton.addEventListener("click", toggleAutoplay);

