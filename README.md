This project is an infiltration AI prototype made using Behaviour Trees
=

Behaviour
-

The entities can either :
* walk through a predefined path, 
* walk to randomized patrol points, 
* or stay static until they notice the player

When the entity reaches a patrol point,
it turns itself to look at the wanted direction and then play an animation to look around for a certain amount of time.
*The direction is the patrol point's transform's forward and the time can be modified for any point*.

Each entity belongs to a squad. When it sees the player,
it warns the other members of its squad so that they come and track the player, and shoot him in the same time.
When all entities of a squad have lost the player, they look for him in the zone around the last point where he was seen.
If the whole zone has been searched and the player is not found, every entity returns to their patrol.

Test level
-

Controls :
* Z,Q,S,D to move
* Left-click to shoot _(it takes about **4 shots** to kill an entity)_

There is a test level in SilentProject/Test_Maps/ with two areas :

The area in front of the player's spawn is filled with blocks blocking visibility.
There are 4 entities, each patrolling randomly in their corner. When one sees the player, they all come and track him.
This zone can be nice to watch the search zone behaviour.

*Tip : In the editor, you can press F1 to see through walls*.

The area behind the player's spawn is an empty area with 4 entities.
Two of them are following a predefined path and the other two are static.
Like in the first area, they all track the player when one sees him.

*This project was made with Unreal Engine 4.23.1, to test it you need to open SP_00.uproject and once in the editor open the test level*