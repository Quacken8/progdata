#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <limits.h>
#define MAX_CAPACITY 100000
#define DOUBLE_TROUBLE 200001

typedef struct Vehicle
{
  long long availibleFrom;
  long long availibleTo;
  long long capacity;
  long long cost;
} Vehicle;

typedef struct VehicleReadRes
{
  bool success;
  long long totalNumber;
  long long smallestFrom;
  long long largestTo;
} VehicleReadRes;

typedef struct Problem
{
  long long day;
  long long pieces;
} Problem;

typedef struct ProblemReadRes
{
  Problem problem;
  bool eof;
  bool success;
} ProblemReadRes;

typedef struct CumulativeWork
{
  long long pieces;
  long long cost;
  long long day;
} CumulativeWork;

static CumulativeWork piecewise[DOUBLE_TROUBLE];
static CumulativeWork integratedPiecewise[DOUBLE_TROUBLE];

bool isValidVehicle(Vehicle v)
{
  return (!(
      v.availibleFrom < 0 || v.availibleTo < 0 || v.availibleFrom > v.availibleTo || v.capacity <= 0 || v.cost <= 0));
}

long long min(long long a, long long b)
{
  if (a < b)
    return a;
  return b;
}

long long max(long long a, long long b)
{
  if (a > b)
    return a;
  return b;
}

int smallerFromFirst(const void *elem1, const void *elem2)
{
  Vehicle a = *((Vehicle *)elem1);
  Vehicle b = *((Vehicle *)elem2);
  if (a.availibleFrom > b.availibleFrom)
    return 1;
  if (a.availibleFrom < b.availibleFrom)
    return -1;
  return 0;
}
int smallerToFirst(const void *elem1, const void *elem2)
{
  Vehicle a = *((Vehicle *)elem1);
  Vehicle b = *((Vehicle *)elem2);
  if (a.availibleTo > b.availibleTo)
    return 1;
  if (a.availibleTo < b.availibleTo)
    return -1;
  return 0;
}

void sortVehicles(Vehicle arr[], long long howMany, int (*fun)(const void *, const void *))
{
  qsort(arr, howMany, sizeof(Vehicle), fun);
}

VehicleReadRes readVehicleInfo(Vehicle *byFrom, Vehicle *byTo)
{
  VehicleReadRes res = {
      .success = false,
      .totalNumber = 0,
      .smallestFrom = INT_MAX,
      .largestTo = INT_MIN,
  };
  printf("Moznosti dopravy:\n");

  char openingBrace;
  // NOLINTNEXTLINE
  if (scanf(" %c", &openingBrace) != 1 || openingBrace != '{')
    return res;

  while (true)
  {
    if (res.totalNumber > MAX_CAPACITY)
      return res;
    Vehicle v;
    char closingChar;
    // NOLINTNEXTLINE
    if (scanf(" [ %lld - %lld , %lld , %lld ] %c", &v.availibleFrom, &v.availibleTo, &v.capacity, &v.cost, &closingChar) != 5)
      return res;
    if (!isValidVehicle(v))
      return res;

    res.largestTo = max(res.largestTo, v.availibleTo);
    res.smallestFrom = min(res.smallestFrom, v.availibleFrom);
    byFrom[res.totalNumber] = v;
    byTo[res.totalNumber] = v;
    res.totalNumber++;

    if (closingChar == '}')
      break;

    if (closingChar != ',' || res.totalNumber == MAX_CAPACITY)
      return res;
  }

  res.success = true;
  return res;
}

ProblemReadRes readProblem()
{
  ProblemReadRes res;
  res.success = false;
  res.eof = false;
  long long startingDay, pieces;
  // NOLINTNEXTLINE
  long long readVals = scanf(" %lld %lld", &startingDay, &pieces);
  if (readVals == EOF)
  {
    res.eof = true;
    return res;
  }
  if (readVals != 2 || startingDay < 0 || pieces <= 0)
  {
    return res;
  }

  Problem problem = {
      .day = startingDay,
      .pieces = pieces,
  };
  res.problem = problem;
  res.success = true;
  return res;
}

long long binaryFindDay(long long query, long long arrayLen, CumulativeWork array[])
{
  long long lower = 0;
  long long upper = arrayLen - 1;

  while (lower < upper)
  {
    long long mid = lower + (upper - lower) / 2;
    long long guess = array[mid].day;
    if (query == guess)
    {
      lower = mid;
      break;
    }
    if (query < guess)
    {
      upper = mid;
    }
    else
    {
      lower = mid + 1;
    }
  }

  return lower;
}

CumulativeWork valueOnDay(long long day, CumulativeWork *integratedPiecewise, CumulativeWork *piecewise, long long len)
{
  long long index = binaryFindDay(day, len, integratedPiecewise);
  if (integratedPiecewise[index].day == day || index == 0)
    return integratedPiecewise[index];

  CumulativeWork dayBefore = integratedPiecewise[index - 1];
  CumulativeWork derivative = piecewise[index - 1];
  long long interval = (day - dayBefore.day);
  CumulativeWork res = {
      .pieces = dayBefore.pieces + derivative.pieces * interval,
      .cost = dayBefore.cost + derivative.cost * interval,
      .day = day,
  };

  return res;
}

CumulativeWork getCumWithPieces(long long pieces, long long arrayLen, CumulativeWork integratedPiecewise[], CumulativeWork piecewise[])
{

  long long lower = integratedPiecewise[0].day;
  long long upper = integratedPiecewise[arrayLen - 1].day;

  while (lower < upper)
  {
    long long mid = lower + (upper - lower) / 2;
    CumulativeWork guess = valueOnDay(mid, integratedPiecewise, piecewise, arrayLen);

    if (pieces < guess.pieces)
    {
      upper = mid;
    }
    else if (pieces > guess.pieces)
    {
      lower = mid + 1;
    }
    else if (lower != upper)
    {
      upper = mid;
    }
  }

  return valueOnDay(lower, integratedPiecewise, piecewise, arrayLen);
}

void solveProblem(Problem problem, long long earliestPossibleDay, long long lastPossibleDay, CumulativeWork *piecewise, CumulativeWork *integratedPiecewise, long long piecewiseLen)
{

  long long day = problem.day < earliestPossibleDay ? earliestPossibleDay : problem.day;

  if (day > lastPossibleDay)
  {
    printf("Prilis velky naklad, nelze odvezt.\n");
    return;
  }

  CumulativeWork beforeFirstDay = valueOnDay(day - 1, integratedPiecewise, piecewise, piecewiseLen);

  CumulativeWork lastDay = getCumWithPieces(beforeFirstDay.pieces + problem.pieces, piecewiseLen, integratedPiecewise, piecewise);

  if ((lastDay.pieces - beforeFirstDay.pieces) < problem.pieces)
  {
    printf("Prilis velky naklad, nelze odvezt.\n");
  }
  else
  {
    printf("Konec: %lld, cena: %lld\n", lastDay.day, lastDay.cost - beforeFirstDay.cost);
  }
};

long long setUpPiecewiseArr(Vehicle *vehiclesByFrom, Vehicle *vehiclesByTo, long long vehiclesLen)
{
  CumulativeWork zeroethKnee = {
      .pieces = 0,
      .cost = 0,
      .day = -1,
  };
  piecewise[0] = zeroethKnee;
  Vehicle fromV = vehiclesByFrom[0], toV = vehiclesByTo[0];

  CumulativeWork yesterdaysRes = piecewise[0];
  long long fromsIndex = 0;
  long long tosIndex = 0;
  long long piecewiseIndex = 1;

  while (tosIndex < vehiclesLen || fromsIndex < vehiclesLen)
  {
    if (tosIndex != vehiclesLen && fromsIndex != vehiclesLen)
      yesterdaysRes.day = min(toV.availibleTo + 1, fromV.availibleFrom);
    else if (tosIndex != vehiclesLen)
      yesterdaysRes.day = toV.availibleTo + 1;
    else
      yesterdaysRes.day = fromV.availibleFrom;

    while (tosIndex != vehiclesLen && (toV.availibleTo + 1) == yesterdaysRes.day)
    {
      yesterdaysRes.cost -= toV.cost;
      yesterdaysRes.pieces -= toV.capacity;
      tosIndex++;
      if (tosIndex == vehiclesLen)
        break;
      toV = vehiclesByTo[tosIndex];
    }

    while (fromsIndex != vehiclesLen && fromV.availibleFrom == yesterdaysRes.day)
    {
      yesterdaysRes.cost += fromV.cost;
      yesterdaysRes.pieces += fromV.capacity;
      fromsIndex++;
      if (fromsIndex == vehiclesLen)
        break;
      fromV = vehiclesByFrom[fromsIndex];
    }

    piecewise[piecewiseIndex] = yesterdaysRes;
    piecewiseIndex++;
  }

  return piecewiseIndex;
}

void integratePieceWise(long long piecewiseLen)
{
  long long lastCost = 0;
  long long lastPieces = 0;
  long long currCost = lastCost;
  long long currPieces = lastPieces;

  for (long long i = 1; i < piecewiseLen; i++)
  {
    long long intervalLen = piecewise[i].day - piecewise[i - 1].day;

    currCost += (intervalLen - 1) * lastCost + piecewise[i].cost;
    currPieces += (intervalLen - 1) * lastPieces + piecewise[i].pieces;
    lastCost = piecewise[i].cost;
    lastPieces = piecewise[i].pieces;

    integratedPiecewise[i].cost = currCost;
    integratedPiecewise[i].pieces = currPieces;
    integratedPiecewise[i].day = piecewise[i].day;
  }
}

int main()
{
  Vehicle vehiclesByFrom[MAX_CAPACITY];
  Vehicle vehiclesByTo[MAX_CAPACITY];
  VehicleReadRes readVehicles = readVehicleInfo(vehiclesByFrom, vehiclesByTo);
  if (!readVehicles.success)
  {
    printf("Nespravny vstup.\n");
    return 1;
  }
  sortVehicles(vehiclesByFrom, readVehicles.totalNumber, smallerFromFirst);
  sortVehicles(vehiclesByTo, readVehicles.totalNumber, smallerToFirst);

  long long pieceWiseLen = setUpPiecewiseArr(vehiclesByFrom, vehiclesByTo, readVehicles.totalNumber);
  integratePieceWise(pieceWiseLen);

  printf("Naklad:\n");
  while (true)
  {
    ProblemReadRes read = readProblem();
    if (read.eof)
    {
      break;
    }
    if (!read.success)
    {
      printf("Nespravny vstup.\n");
      return 2;
    }

    solveProblem(read.problem, readVehicles.smallestFrom, readVehicles.largestTo, piecewise, integratedPiecewise, pieceWiseLen);
  }

  return 0;
}