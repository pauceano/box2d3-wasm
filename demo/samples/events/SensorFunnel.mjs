import Sample from "../sample.mjs";

import camera from '../camera.mjs';
import settings from '../settings.mjs';

export default class SensorFunnel extends Sample{
	constructor(box2d, canvas){
		super(box2d, canvas);

		camera.center = {x: 0.0, y: 0.0 };
		camera.zoom = 25.0 * 1.333;

		settings.drawJoints = false;
		/*
		{
			b2BodyDef bodyDef = b2DefaultBodyDef();
			b2BodyId groundId = b2CreateBody( m_worldId, &bodyDef );


			b2Vec2 points[] = {
				{ -16.8672504, 31.088623 },	 { 16.8672485, 31.088623 },		{ 16.8672485, 17.1978741 },
				{ 8.26824951, 11.906374 },	 { 16.8672485, 11.906374 },		{ 16.8672485, -0.661376953 },
				{ 8.26824951, -5.953125 },	 { 16.8672485, -5.953125 },		{ 16.8672485, -13.229126 },
				{ 3.63799858, -23.151123 },	 { 3.63799858, -31.088623 },	{ -3.63800049, -31.088623 },
				{ -3.63800049, -23.151123 }, { -16.8672504, -13.229126 },	{ -16.8672504, -5.953125 },
				{ -8.26825142, -5.953125 },	 { -16.8672504, -0.661376953 }, { -16.8672504, 11.906374 },
				{ -8.26825142, 11.906374 },	 { -16.8672504, 17.1978741 },
			};

			int count = std::size( points );

			b2ChainDef chainDef = b2DefaultChainDef();
			chainDef.points = points;
			chainDef.count = count;
			chainDef.isLoop = true;
			chainDef.friction = 0.2f;
			b2CreateChain( groundId, &chainDef );

			float sign = 1.0f;
			float y = 14.0f;
			for ( int i = 0; i < 3; ++i )
			{
				bodyDef.position = { 0.0f, y };
				bodyDef.type = b2_dynamicBody;

				b2BodyId bodyId = b2CreateBody( m_worldId, &bodyDef );

				b2Polygon box = b2MakeBox( 6.0f, 0.5f );
				b2ShapeDef shapeDef = b2DefaultShapeDef();
				shapeDef.friction = 0.1f;
				shapeDef.restitution = 1.0f;
				shapeDef.density = 1.0f;

				b2CreatePolygonShape( bodyId, &shapeDef, &box );

				b2RevoluteJointDef revoluteDef = b2DefaultRevoluteJointDef();
				revoluteDef.bodyIdA = groundId;
				revoluteDef.bodyIdB = bodyId;
				revoluteDef.localAnchorA = bodyDef.position;
				revoluteDef.localAnchorB = b2Vec2_zero;
				revoluteDef.maxMotorTorque = 200.0f;
				revoluteDef.motorSpeed = 2.0f * sign;
				revoluteDef.enableMotor = true;

				b2CreateRevoluteJoint( m_worldId, &revoluteDef );

				y -= 14.0f;
				sign = -sign;
			}

			{
				b2Polygon box = b2MakeOffsetBox( 4.0f, 1.0f, { 0.0f, -30.5f }, b2Rot_identity );
				b2ShapeDef shapeDef = b2DefaultShapeDef();
				shapeDef.isSensor = true;
				b2CreatePolygonShape( groundId, &shapeDef, &box );
			}
		}

		m_wait = 0.5f;
		m_side = -15.0f;
		m_type = e_human;

		for ( int i = 0; i < e_count; ++i )
		{
			m_isSpawned[i] = false;
		}

		memset( m_humans, 0, sizeof( m_humans ) );

		CreateElement();*/
	}
}
