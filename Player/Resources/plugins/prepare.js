var Agtk = {};
Agtk.constants = {};
Agtk.constants.actionCommands = {};
Agtk.constants.actionCommands.ObjectByType = 0;
Agtk.constants.actionCommands.ObjectByGroup = 0;
Agtk.constants.actionCommands.ObjectById = 1;

Agtk.constants.actionCommands.ProjectCommon = 0;
Agtk.constants.actionCommands.UnsetObject = -1;
Agtk.constants.actionCommands.SelfObject = -2;
Agtk.constants.actionCommands.OtherThanSelfObject = -3;
Agtk.constants.actionCommands.ChildObject = -4;
Agtk.constants.actionCommands.LockedObject = -5;
Agtk.constants.actionCommands.TouchedObject = -6;
Agtk.constants.actionCommands.ParentObject = -7;

Agtk.constants.actionCommands.AccordingToMoveDirection = -2;

Agtk.constants.actionCommands.commandBehavior = {
    CommandBehaviorNext: 0,
    CommandBehaviorLoop: 1,
    CommandBehaviorBlock: 2,
    CommandBehaviorBreak: 3
};
Agtk.constants.actionCommands.priorityType = {
    PriorityBackground: 0,
    PriorityMostFront: 1,
    PriorityMostFrontWithMenu: 2
};

Agtk.constants.actionCommands.templateMove = {
    MoveHorizontal: 0,
    MoveVertical: 1,
    MoveBound: 2,
    MoveRandom: 3,
    MoveNearObject: 4,
    MoveNearPlayer: 4,  //deprecated
    MoveApartNearObject: 5,
    MoveApartNearPlayer: 5, //deprecated
    MoveStop: 6
};
Agtk.constants.actionCommands.objectLock = {
    SetByObjectGroup: 0,
    SetByObject: 1,

    UseSwitch: 0,
    UseVariable: 1,
    UseNone: 2,

    CompareValueConstant: 0,
    CompareValueOtherVariable: 1,
    CompareValueNonNumeric: 2
};
Agtk.constants.actionCommands.objectCreate = {
    PositionCenter: 0,
    PositionLockObjectCenter: 1
};
Agtk.constants.actionCommands.objectChange = {
    PositionCenter: 0,
    PositionLockObjectCenter: 1
};
Agtk.constants.actionCommands.objectMove = {
    MoveWithDirection: 0,
    MoveToPosition: 1,
    MoveToObjectCenter: 2,
    MoveToObjectOrigin: 3,
    MoveToObjectConnectionPoint: 4,

    TargettingByType: 0,
    TargettingByGroup: 0,
    TargettingById: 1,
    TargettingTouched: 2,
    TargettingLocked: 3
};
Agtk.constants.actionCommands.objectPushPull = {
    DirectionAngle: 0,
    DirectionObjectDisp: 1,

    EffectDirectionAngle: 0,
    EffectDirectionObjectDisp: 1,
    EffectDirectionObjectConnect: 2,

    TargettingByType: 0,
    TargettingByGroup: 0,
    TargettingById: 1,
    TargettingTouched: 2,
    TargettingLocked: 3
};
Agtk.constants.actionCommands.attackSetting = {
    AttributeNone: 0,
    AttributePreset: 1,
    AttributeValue: 2
};
Agtk.constants.actionCommands.sceneRotateFlip = {
    TypeReset: 0,
    TypeRotationFlip: 1
};
Agtk.constants.actionCommands.soundPlay = {
    SoundSe: 0,
    SoundVoice: 1,
    SoundBgm: 2
};
Agtk.constants.actionCommands.soundStop = {
    SoundSe: 0,
    SoundVoice: 1,
    SoundBgm: 2
};
Agtk.constants.actionCommands.soundPositionRemember = {
    SoundSe: 0,
    SoundVoice: 1,
    SoundBgm: 2
};
Agtk.constants.actionCommands.messageShow = {
    WindowNone: -1,
    WindowTemplate: 0,
    WindowImage: 1,

    TemplateWhiteFrame: 0,
    TemplateBlack: 1,
    TemplateWhite: 2,

    PositionCenter: 0,
    PositionLockObjectCenter: 1,
    PositionScenePosition: 2,

    HorzAlignLeft: 0,
    HorzAlignCenter: 1,
    HorzAlignRight: 2,

    VertAlignTop: 0,
    VertAlignCenter: 1,
    VertAlignBottom: 2
};
Agtk.constants.actionCommands.scrollMessageShow = {
    BackgroundNone: -1,
    BackgroundTemplate: 0,
    BackgroundImage: 1,

    TemplateBlack: 1,
    TemplateWhite: 2,

    PositionCenter: 0,
    PositionLockObjectCenter: 1,
    PositionScenePosition: 2,

    HorzAlignLeft: 0,
    HorzAlignCenter: 1,
    HorzAlignRight: 2
};
Agtk.constants.actionCommands.effectShow = {
    PositionCenter: 0,
    PositionLockObjectCenter: 1
};
Agtk.constants.actionCommands.effectRemove = {
    AllEffects: -2,

    TargettingByType: 0,
    TargettingByGroup: 0,
    TargettingById: 1,
    TargettingSceneEffect: 5
};
Agtk.constants.actionCommands.particleShow = {
    PositionCenter: 0,
    PositionLockObjectCenter: 1
};
Agtk.constants.actionCommands.particleRemove = {
    AllParticles: -2,

    TargettingByType: 0,
    TargettingByGroup: 0,
    TargettingById: 1,
    TargettingSceneParticle: 5
};
Agtk.constants.actionCommands.movieShow = {
    PositionCenter: 0,
    PositionLockObjectCenter: 1,
    PositionScenePosition: 2,

    VertAlignCenter: 0,
    VertAlignTop: 1,
    VertAlignBottom: 2,

    HorzAlignCenter: 0,
    HorzAlignLeft: 1,
    HorzAlignRight: 2
};
Agtk.constants.actionCommands.imageShow = {
    PositionCenter: 0,
    PositionLockObjectCenter: 1,
    PositionScenePosition: 2,
    kPositionCenter: 0,
    kPositionLockObjectCenter: 1,
    kPositionScenePosition: 2,

    VertAlignCenter: 0,
    VertAlignTop: 1,
    VertAlignBottom: 2,
    kVertAlignCenter: 0,
    kVertAlignTop: 1,
    kVertAlignBottom: 2,

    HorzAlignCenter: 0,
    HorzAlignLeft: 1,
    HorzAlignRight: 2,
    kHorzAlignCenter: 0,
    kHorzAlignLeft: 1,
    kHorzAlignRight: 2
};
Agtk.constants.actionCommands.gameSpeedChange = {
    TargettingByType: 0,
    TargettingByGroup: 0,
    TargettingById: 1,
    TargettingTouched: 2,
    TargettingLocked: 3
};
Agtk.constants.actionCommands.timer = {
    SecondByValue: 0,
    SecondByVariable: 1
};
Agtk.constants.actionCommands.directionMove = {
    AccordingToMoveDirection: -2
};
Agtk.constants.actionCommands.forthBackMoveTurn = {
    MoveNone: 0,
    MoveForth: 1,
    MoveBack: 2,
    kMoveNone: 0,
    kMoveForth: 1,
    kMoveBack: 2,

    TurnNone: 0,
    TurnRight: 1,
    TurnLeft: 2,
    kTurnNone: 0,
    kTurnRight: 1,
    kTurnLeft: 2,

    AccordingToMoveDirection: -2
};
Agtk.constants.actionCommands.menuShow = {
    None: -1,
    SlideUp: 0,
    SlideDown: 1,
    SlideLeft: 2,
    SlideRight: 3
};
Agtk.constants.actionCommands.menuHide = {
    None: -1,
    SlideUp: 0,
    SlideDown: 1,
    SlideLeft: 2,
    SlideRight: 3
};
Agtk.constants.actionCommands.fileLoad = {
    None: -1,
    Black: 0,
    White: 1,
    SlideUp: 2,
    SlideDown: 3,
    SlideLeft: 4,
    SlideRight: 5
};
Agtk.constants.actionCommands.objectUnlock = {
    SetByObjectGroup: 0,
    SetByObjectId: 1
};

Agtk.constants.linkCondition = {};
Agtk.constants.linkCondition.objectWallTouched = {
    SetByObjectGroup: 0,
    SetByObjectId: 1
};
Agtk.constants.linkCondition.objectHit = {
    SetByObjectGroup: 0,
    SetByObjectId: 1
};
Agtk.constants.linkCondition.attackAreaTouched = {
    SetByObjectGroup: 0,
    SetByObjectId: 1,

    AttributeNone: 0,
    AttributePreset: 1,
    AttributeValue: 2
};
Agtk.constants.linkCondition.attackAreaNear = {
    DistanceNone: 0,
    DistanceGreaterEqual: 1,
    DistanceLessEqual: 2,

    SetByObjectGroup: 0,
    SetByObjectId: 1,

    AttributeNone: 0,
    AttributePreset: 1,
    AttributeValue: 2
};
Agtk.constants.linkCondition.objectNear = {
    DistanceNone: 0,
    DistanceGreaterEqual: 1,
    DistanceLessEqual: 2,

    SetByObjectGroup: 0,
    SetByObjectId: 1
};
Agtk.constants.linkCondition.objectFacingEachOther = {
    SetByObjectGroup: 0,
    SetByObjectId: 1
};
Agtk.constants.linkCondition.objectFacing = {
    SetByObjectGroup: 0,
    SetByObjectId: 1
};
Agtk.constants.linkCondition.objectFound = {
    SetByObjectGroup: 0,
    SetByObjectId: 1
};
Agtk.constants.linkCondition.objectFacingDirection = {
    SetByObjectGroup: 0,
    SetByObjectId: 1
};
Agtk.constants.linkCondition.locked = {
    SetByObjectGroup: 0,
    SetByObjectId: 1
};
Agtk.constants.linkCondition.slopeTouched = {
    DirectionUpper: 0,
    DirectionLower: 1,
    DirectionAny: 2,

    DownwardLeft: 0,
    DownwardRight: 1,
    DownwardNone: 2
};

Agtk.constants.conditions = {
    SwitchConditionOn: 0,
    SwitchConditionOff: 1,
    SwitchConditionOnFromOff: 2,
    SwitchConditionOffFromOn: 3,

    OperatorLess: 0,
    OperatorLessEqual: 1,
    OperatorEaual: 2,
    OperatorGreaterEqual: 3,
    OperatorGreater: 4,
    OperatorNotEqual: 5,

    CompareValue: 0,
    CompareVariable: 1,
    CompareNaN: 2
};
Agtk.constants.assignments = {
    SwitchAssignOn: 0,
    SwitchAssignOff: 1,
    SwitchAssignToggle: 2,

    VariableAssignOperatorSet: 0,
    VariableAssignOperatorAdd: 1,
    VariableAssignOperatorSub: 2,
    VariableAssignOperatorMul: 3,
    VariableAssignOperatorDiv: 4,
    VariableAssignOperatorMod: 5,

    VariableAssignValue: 0,
    VariableAssignVariable: 1,
    VariableAssignRandom: 2,
    VariableAssignScript: 3,
};
Agtk.constants.attackAttributes = {
    Fire: 1,
    Water: 2,
    Earth: 3,
    Wind: 4,
    Lightning: 5,
    Ice: 6,
    Light: 7,
    Dark: 8
};
Agtk.constants.filterEffects = {
    EffectNoise: 0,
    EffectMosaic: 1,
    EffectMonochrome: 2,
    EffectSepia: 3,
    EffectNegaPosiReverse: 4,
    EffectDefocus: 5,
    EffectChromaticAberration: 6,
    EffectDarkness: 7,
    EffectDispImage: 8,
    EffectFillColor: 9,
    EffectTransparency: 10,
    EffectBlink: 11,

    PlacementCenter: 0,
    PlacementMagnify: 1,
    PlacementTiling: 2,
    PlacementKeepRatio: 3,
    PlacementObjectCenter: 4
};
Agtk.constants.systemLayers = {
    SystemLayerAllId: -1,
    SystemLayerBackgroundId: -2,
    HudLayerId: 9999
};
Agtk.constants.qualifier = {
    QualifierSingle: -1,
    QualifierWhole: -2
};

Agtk.constants.objectType = {
    ObjectTypeAll: 0,
    ObjectTypePlayer: 1,
    ObjectTypeEnemy: 2
};

Agtk.constants.objectGroup = {
    ObjectGroupAll: -1,
    ObjectGroupPlayer: 0,
    ObjectGroupEnemy: 1
};

Agtk.constants.tileGroup = {
    TileGroupDefault: 0
};

Agtk.constants.tile = {
    WallTop: 0,
    WallLeft: 1,
    WallRight: 2,
    WallBottom: 3,

    WallTopBit: (1 << 0),
    WallLeftBit: (1 << 1),
    WallRightBit: (1 << 2),
    WallBottomBit: (1 << 3),
    WallBitMask: 0x0f
};
Agtk.constants.direction = {
    BottomLeftBit: (1 << 1),
    BottomBit: (1 << 2),
    BottomRightBit: (1 << 3),
    LeftBit: (1 << 4),

    RightBit: (1 << 6),
    TopLeftBit: (1 << 7),
    TopBit: (1 << 8),
    TopRightBit: (1 << 9),

    AllDirectionBit: 0x3DE
};

Agtk.constants.controllers = {
    OperationKeyA: 1,
    OperationKeyB: 2,
    OperationKeyX: 3,
    OperationKeyY: 4,
    OperationKeyR1: 5,
    OperationKeyR2: 6,
    OperationKeyL1: 7,
    OperationKeyL2: 8,
    OperationKeyUp: 9,
    OperationKeyDown: 10,
    OperationKeyLeft: 11,
    OperationKeyRight: 12,
    OperationKeyLeftStickUp: 13,
    OperationKeyLeftStickDown: 14,
    OperationKeyLeftStickLeft: 15,
    OperationKeyLeftStickRight: 16,
    OperationKeyRightStickUp: 17,
    OperationKeyRightStickDown: 18,
    OperationKeyRightStickLeft: 19,
    OperationKeyRightStickRight: 20,
    OperationKeyLeftClick: 21,
    OperationKeyRightClick: 22,
    OperationKeyStart: 23,
    OperationKeySelect: 24,
    OperationKeyHome: 25,
    OperationKeyOk: 26,
    OperationKeyCancel: 27,

    ReservedKeyCodePc_W: 0,
    ReservedKeyCodePc_A: 1,
    ReservedKeyCodePc_S: 2,
    ReservedKeyCodePc_D: 3,
    ReservedKeyCodePc_LeftClick: 4,
    ReservedKeyCodePc_RightClick: 5,
    ReservedKeyCodePc_Up: 10,
    ReservedKeyCodePc_Right: 11,
    ReservedKeyCodePc_Down: 12,
    ReservedKeyCodePc_Left: 13,
    ReservedKeyCodePc_MiddleClick: 22,
    ReservedKeyCodePc_WheelUp: 24,
    ReservedKeyCodePc_WhellDown: 26,
    ReservedKeyCodePc_MousePointer: 28,
};

Agtk.constants.animations = {
    Motion: 0,
    Effect: 1,
    Particle: 2
};

Agtk.constants.tracks = {
    TimelineWall: 0,
    TimelineHit: 1,
    TimelineAttack: 2,
    TimelineConnect: 3
};

Agtk.constants.objects = {
    switches: {
        InvincibleId: 1,
        FreeMoveId: 2,
        LockTargetId: 3,
        PortalTouchedId: 4,
        CriticalDamagedId: 5,
        DisabledId: 6,
        SlipOnSlopeId: 7,
        AffectOtherObjectsId: 8,
        AffectedByOtherObjectsId: 9,
        FollowConnectedPhysicsId: 10,
        DisplayAfterimageId: 11
    },
    variables: {
        ObjectIDId: 1,
        HPId: 2,
        MaxHPId: 3,
        MinimumAttackId: 4,
        MaximumAttackId: 5,
        DamageRatioId: 6,
        AttackAttributeId: 7,
        AreaAttributeId: 8,
        XId: 9,
        YId: 10,
        VelocityXId: 11,
        VelocityYId: 12,
        PlayerIDId: 13,
        DamageValueId: 14,
        CriticalRatioId: 15,
        CriticalIncidenceId: 16,
        InvincibleDurationId: 17,
        FixedAnimationFrameId: 18,
        InstanceIDId: 19,
        InstanceCountId: 20,
        SingleInstanceIDId: 21,
        ControllerIDId: 22,
        HorizontalMoveId: 23,
        VerticalMoveId: 24,
        HorizontalAccelId: 25,
        VerticalAccelId: 26,
        HorizontalMaxMoveId: 27,
        VerticalMaxMoveId: 28,
        HorizontalDecelId: 29,
        VerticalDecelId: 30,
        DurationToTakeOverAccelerationMoveSpeedId: 31,
        ScalingXId: 32,
        ScalingYId: 33,
        DispPriorityId: 34,
        InitialJumpSpeedId: 35,
        DamageVariationValueId: 36,
        DisplayDirectionId: 37,
        ParentObjectInstanceIDId: 38
    }
};

Agtk.constants.switchVariableObjects = {
    ProjectCommon: 0,
    SelfObject: -2,
    ParentObject: -7
};

Agtk.constants.databaseTemplateTypes = {
    Default: 0,
    Object: 1
};